#pragma once
#include <queue>
#include <stack>
#include <unordered_map>
#include "RuntimeFailureReason.h"
#include "FrameworkEntity.h"
#include "behavior/Sequence.h"
#include "services/interfaces/IContentProvider.h"
#include "services/interfaces/IEntityQuery.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IStartCharacterActionProvider.h"
#include "services/interfaces/IStateOperationEvaluator.h"
#include "services/interfaces/ITimeManager.h"
#include "memory/MemorySystem.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Behavioral Entities are characters with autonomous decision-making capabilities.
 *
 * This class orchestrates the core behavioral execution loop for ambient NPCs, integrating:
 * - Memory-driven action/transition selection for behavioral variety
 * - Hierachical sequence execution with interruption support
 * - Context-aware entity selection and precondition validation for actions and transitions
 * - Resilient failure recovery through fallback sequences
 *
 * EXECUTION FLOW: Characters continuously execute their current sequences via ExecuteCurrentSequence(), which
 * dispatches to specialized handlers based on SequenceState. See ExecuteSequenceStep() for the main state machine.
 */
class BehavioralEntity : public FrameworkEntity {
// ══════════════════════════════════════════════════════════════════════════
// CORE STATE & DEPENDENCIES
// ═══════════════════════════════════════════════════════════════════════════
private:
    // Provides access to framework-wide services
    ContentRegistryServices& services;

    MemorySystem memory;
    std::shared_ptr<Sequence> main_sequence;
    std::stack<std::shared_ptr<Sequence>> sequences;

    std::vector<std::shared_ptr<Sequence>> fallback_sequences;
    std::unordered_map<int32_t, std::shared_ptr<Sequence>> interruption_handlers;

    //-------- Action execution context --------------

    // The entity being used in the current action (-1 if none)
    int32_t current_action_target_id;

    // The currently executing action (-1 if none)
    int32_t current_action_id;

    // Prevents late callbacks from stale actions
    int64_t current_action_token;

    //--------- Flow control ---------------

    // Prevents reentrant ExecuteCurrentSequence calls
    bool is_processing;

    // Set after max fallback failures. Stops updates to the character
    bool is_halted;

    // Tracks consecutive fallback attempts
    int32_t fallback_attempt_count;

    // Prevents infinite fallback loops
    const int32_t MAX_FALLBACK_ATTEMPTS = 3;

    std::queue<int32_t> pending_interruptions;

    // Service accessors

    [[nodiscard]]
    FoundationServices& FoundationServices() const
    {
        return services.behavioral_evaluation.simulation_state.data_access.foundation;
    }

    [[nodiscard]]
    ILogger& Logger() const
    {
        return FoundationServices().logger;
    }

    [[nodiscard]]
    ITimeManager& TimeManager() const
    {
        return FoundationServices().time_manager;
    }

    [[nodiscard]]
    IStartCharacterActionProvider& ActionProvider() const
    {
        return FoundationServices().start_character_action_provider;
    }

    [[nodiscard]]
    IStateOperationEvaluator& StateEvaluator() const
    {
        return services.behavioral_evaluation.state_operation_evaluator;
    }

    [[nodiscard]]
    IContentProvider& ContentProvider() const
    {
        return services.content_provider;
    }

    [[nodiscard]]
    IEntityQuery& EntityQuery() const
    {
        return services.entity_query;
    }

    [[nodiscard]]
    IActionTimeoutManager& ActionTimeoutManager() const
    {
        return services.behavioral_evaluation.simulation_state.action_timeout_manager;
    }

// ═══════════════════════════════════════════════════════════════════════════
// CONSTRUCTION & CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════
public:
    /**
    *  @brief Creates a behavioral entity with configured memory limits.
     * @param services Framework service registry
     * @param entity_handle Game engine-side handle for this character
     * @param entity_id Unique identifier
     * @param max_transition_memories Capacity for transition history
     * @param max_action_memories Capacity for action-entity pair history
     * @param max_interruption_memories Capacity for interruption context storage
     * @param name (Optional) The name of the character. Used mostly for logging purposes.
     * @throw std::invalid_argument if entity_id < 0.
     */
    explicit BehavioralEntity(
        ContentRegistryServices& services,
        void* entity_handle,
        int32_t entity_id,
        int32_t max_transition_memories,
        int32_t max_action_memories,
        int32_t max_interruption_memories,
        std::string name = ""
    );

    /**
     * @brief Sets the character's primary behavioral pattern.
     * Creates a fresh instance if different from current main sequence.
     */
    void SetMainSequence(const std::shared_ptr<Sequence> &new_sequence);

    /**
     * @brief Registers a fallback sequence for failure recovery.
     * Fallback sequences are randomly selected when primary sequences fail.
     */
    void AddFallbackSequence(const std::shared_ptr<Sequence> &new_sequence);

    /**
     * @brief Maps an interruption type to its response sequence.
     * When ProcessInterruption(id) is called, the corresponding sequence is pushed.
     */
    void AddInterruptionHandler(int32_t interruption_id, const std::shared_ptr<Sequence> &handler);

// ═══════════════════════════════════════════════════════════════════════════
// MAIN EXECUTION LOOP
// ═══════════════════════════════════════════════════════════════════════════
public:

    /**
     * @brief Main execution entry point. Called each framework update.
     *
     * EXECUTION STRATEGY:
     * 1. Process any pending interruptions first (queued during processing)
     * 2. Execute one step of current sequence based on its SequenceState
     * 3. Set is_processing flag to prevent re-entrant calls
     *
     * State machine dispatching happens in ExecuteSequenceStep().
     *
     * @note This method is idempotent so it's safe to call multiple times per frame.
     *       The is_processing flag ensures only one execution path runs.
     */
    void ExecuteCurrentSequence();

    /**
     * @brief Determines if character can process an update.
     *
     * Characters cannot update if:
     * - Already processing (prevents re-entrance)
     * - Waiting for action completion (waiting for callback)
     * - Halted after max fallback attempts (terminal failure state)
     */
    [[nodiscard]]
    bool CanUpdate() const;

private:

    /**
     * @brief Core state machine. Dispatches to handlers based on SequenceState.
     *
     * STATE TRANSITIONS:
     * - UNINITIALIZED     -> HandleSequenceStartup() -> PROCESSING_NODE
     * - PROCESSING_NODE   -> ProcessCurrentNode() -> [varies by node type]
     * - IN_SUBSEQUENCE    -> HandleSubsequenceCompletion() -> NODE_EXECUTED
     * - WAITING_FOR_ACTION -> [no operation, waits for CompleteAction callback]
     * - NODE_EXECUTED     -> HandleNodeExecutionCompletion() -> PROCESSING_NODE
     * - FAILED            -> HandleSequenceFailure() -> UNINITIALIZED (fallback)
     * - INTERRUPTED       -> HandleInterruptionRecovery() -> [attempts resumption]
     *
     * @param sequence_state Current state of the top sequence on the stack
     */
    void ExecuteSequenceStep(SequenceState sequence_state);

    /**
     * @brief Handles empty sequence stack by pushing the main sequence.
     * Terminal condition if main_sequence is null (sets is_processing=true to halt).
     */
    void HandleEmptySequences();

    /**
     * @brief Initializes a new sequence for execution.
     * Resets to entry node, clears fallback attempts, transitions to PROCESSING_NODE.
     */
    void HandleSequenceStartup();

// ═══════════════════════════════════════════════════════════════════════════
// NODE PROCESSING (Sequence Step Execution)
// ═══════════════════════════════════════════════════════════════════════════
private:
    /**
     * @brief Processes the current node of the active sequence.
     *
     * Dispatches to specialized executors based on node type:
     * - ACTION_NODE -> ExecuteActionNode()
     * - NESTED_SEQUENCE_NODE -> ExecuteNestedSequenceNode()
     * - END_SEQUENCE_NODE -> ExecuteEndSequenceNode()
     */
    void ProcessCurrentNode();

    /**
     * @brief Dispatches node execution based on its concrete type.
     * @param current_node The node to execute (guaranteed non-null)
     */
    void ExecuteCurrentNode(const SequenceNode* current_node);

    /**
     * @brief Executes an action node.
     *
     * EXECUTION STEPS:
     * 1. Lookup action from node's target_action_id
     * 2. Validate SELF and ENVIRONMENT preconditions (no entity context)
     * 3. Acquire target entity if required (memory-driven selection)
     * 4. Initiate action execution (applies immediate effects, starts action)
     *
     * Any failure triggers HandleRuntimeFailure() → FAILED state.
     */
    void ExecuteActionNode(const SequenceNode* current_node);

    /**
     * @brief Pushes a nested sequence onto the execution stack.
     * Parent sequence transitions to IN_SUBSEQUENCE state.
     */
    void ExecuteNestedSequenceNode(const SequenceNode* current_node);

    /**
     * @brief Handles sequence completion - pops stack, may push main sequence.
     * Transitions to NODE_EXECUTED to continue parent sequence.
     */
    void ExecuteEndSequenceNode(const SequenceNode* current_node);

// ═══════════════════════════════════════════════════════════════════════════
// ACTION EXECUTION LIFECYCLE
// ═══════════════════════════════════════════════════════════════════════════
private:
    /**
     * @brief Resolves action object from current action node.
     * @return Action instance or nullptr if lookup fails
     */
    std::shared_ptr<Action> LookupActionFromCurrentNode(const SequenceNode* current_node) const;

    /**
     * @brief Begins action execution with selected entity context.
     *
     * EXECUTION SEQUENCE:
     * 1. Apply immediate effects (state changes before action starts)
     * 2. Record action in memory (for future entity selection)
     * 3. Increment action token (invalidates old callbacks)
     * 4. Call engine's StartCharacterAction() (animation, pathfinding, etc.)
     * 5. Register timeout (framework completes action if engine doesn't)
     * 6. Transition to WAITING_FOR_ACTION state
     *
     * @param action The action to execute
     * @param target_entity Entity to perform action on (nullptr if self-targeted)
     */
    void InitiateActionExecution(const std::shared_ptr<Action>& action, FrameworkEntity* target_entity);

    /**
     * @brief Applies state operations (immediate or completion effects).
     * Evaluates each StateOperation against character and target entity.
     */
    void ApplyActionEffects(const std::vector<StateOperation> & effects, FrameworkEntity* target_entity);

public:
    /**
     * @brief Callback from engine signaling action completion.
     *
     * VALIDATION:
     * - Checks action_id and action_token match current action
     * - Rejects late callbacks from interrupted/superseded actions
     *
     * ON SUCCESS:
     * - Applies completion effects
     * - Unregisters timeout
     * - Transitions to NODE_EXECUTED
     * - Clears is_processing flag
     *
     * @param action_id The completed action's ID
     * @param action_token Token from when action was initiated
     */
    void CompleteAction(int32_t action_id, int64_t action_token);

private:

    /**
     * @brief Applies state changes when action completes.
     * Looks up action, resolves target entity, applies completion effects.
     */
    void ApplyCompletionEffects(int32_t action_id);

    /**
     * @brief Validates that completed action matches current execution context.
     * Prevents late callbacks from stale actions after interruptions.
     */
    [[nodiscard]]
    bool CompletedCurrentAction(int32_t action_id, int64_t action_token) const;

// ═══════════════════════════════════════════════════════════════════════════
// MEMORY-DRIVEN SELECTION (Exploration-Exploitation)
// ═══════════════════════════════════════════════════════════════════════════
private:

    /**
     * @brief Selects target entity for action using memory-driven algorithm.
     *
     * SELECTION PROCESS:
     * 1. Query all entities supporting the action
     * 2. Filter by ENTITY and DISTANCE_TO_ENTITY preconditions
     * 3. Apply memory-based selection:
     *    - Prefer unused entities (exploration)
     *    - Among used, select least recently used (exploitation)
     *    - Random tiebreaker
     *
     * @return Selected entity or nullptr if none valid
     */
    FrameworkEntity* GetActionTargetEntity(const std::shared_ptr<Action>& action);

    /**
     * @brief Selects next node using memory-driven transition selection.
     *
     * SELECTION PROCESS:
     * 1. Get all transitions from current node
     * 2. Filter by SELF and ENVIRONMENT preconditions
     * 3. Apply memory-based selection (same algorithm as entity selection)
     *
     * @return Node ID to transition to, or nullopt if no valid transitions
     */
    std::optional<int32_t> GetNodeIdForNextTransition();

// ═══════════════════════════════════════════════════════════════════════════
// PRECONDITION VALIDATION
// ═══════════════════════════════════════════════════════════════════════════
private:
    /**
     * @brief Validation result with optional failure context.
     */
    struct PreconditionValidation {
        bool precondition_passed;
        std::optional<StateOperationTarget> failed_target;

        [[nodiscard]]
        bool Passed() const { return precondition_passed; }
    };

    /**
     * @brief Validates action preconditions against current context.
     *
     * VALIDATION ORDER:
     * 1. SELF preconditions (character's own state)
     * 2. ENVIRONMENT preconditions (cached environmental conditions)
     * 3. ENTITY preconditions (target entity's state, if provided)
     *
     * @param target_entity Optional - only needed for ENTITY precondition checks
     * @return Validation result indicating pass/fail and which target failed
     */
    PreconditionValidation ValidateActionPreconditions(const std::shared_ptr<Action>& action,
        FrameworkEntity* target_entity = nullptr);

    /**
     * @brief Evaluates a list of preconditions (StateOperations).
     * @param other Entity to use as 'other' in StateOperationContext (nullable)
     * @return true if ALL preconditions evaluate to true
     */
    bool EvaluatePreconditions(const std::vector<StateOperation>* preconditions, FrameworkEntity* other);

// ═══════════════════════════════════════════════════════════════════════════
// INTERRUPTION HANDLING
// ═══════════════════════════════════════════════════════════════════════════
public:

    /**
     * @brief External interruption trigger (called by framework).
     *
     * QUEUEING STRATEGY:
     * - If currently processing: queue for later (prevents corruption)
     * - Otherwise: process immediately
     *
     * Queued interruptions are processed at start of next ExecuteCurrentSequence().
     */
    void ProcessInterruption(int32_t interruption_id);

private:
    /**
     * @brief Processes all queued interruptions in FIFO order.
     * Called at start of ExecuteCurrentSequence() if queue non-empty.
     */
    void ProcessPendingInterruptions();

    /**
     * @brief Immediately processes an interruption.
     *
     * EXECUTION STEPS:
     * 1. Verify handler exists for interruption_id
     * 2. If in WAITING_FOR_ACTION state:
     *    - Apply interruption effects
     *    - Save context if action is RESUMABLE
     *    - Invalidate action token (reject late callbacks)
     * 3. Mark current sequence as INTERRUPTED
     * 4. Push interruption response sequence onto stack
     *
     * @param interruption_id The interruption type to handle
     */
    void ProcessInterruptionImmediate(int32_t interruption_id);

    /**
     * @brief Attempts to resume interrupted sequence after interruption completes.
     *
     * RESUMPTION LOGIC:
     * 1. If interrupted at action node with RESUMABLE behavior:
     *    - Check if interruption memory exists
     *    - Validate context still valid (preconditions, entity availability)
     *    - Resume with saved entity OR transition to PROCESSING_NODE
     * 2. Otherwise: just continue to PROCESSING_NODE
     *
     * This allows characters to "pick up where they left off" when possible.
     */
    void HandleInterruptionRecovery();

    /**
     * @brief Attempts to resume a specific action using saved context.
     *
     * CONTEXT VALIDATION:
     * - Checks all preconditions still satisfied
     * - Verifies target entity still available (if required)
     * - If valid: resumes action with saved entity
     * - If invalid: removes memory, transitions to PROCESSING_NODE for fresh execution
     */
    void AttemptActionResumption(const std::shared_ptr<Action>& action);

    /**
     * @brief Validates that interruption context is still valid for resumption.
     * Checks SELF, ENVIRONMENT, ENTITY, and DISTANCE_TO_ENTITY preconditions.
     */
    bool ValidateResumptionContext(const std::shared_ptr<Action>& action, int32_t target_entity_id);

    /**
     * @brief Resumes action execution using preserved interruption context.
     * Re-initiates action with the same target entity that was interrupted.
     */
    void ResumeActionWithSavedContext(const std::shared_ptr<Action>& action, const InterruptionMemory* interruption_memory);

// ═══════════════════════════════════════════════════════════════════════════
// FAILURE RECOVERY & TRANSITION COMPLETION
// ═══════════════════════════════════════════════════════════════════════════
private:
    /**
     * @brief Completes node execution and transitions to next node.
     *
     * TRANSITION PROCESS:
     * 1. Abort if sequence was interrupted during node execution
     * 2. Find valid transitions using GetNodeIdForNextTransition()
     * 3. Select next node via memory-driven selection
     * 4. Record transition in memory
     * 5. Mark current node as completed
     * 6. Set new current_node
     * 7. Transition to PROCESSING_NODE
     *
     * If no valid transitions exist → triggers HandleSequenceFailure()
     */
    void HandleNodeExecutionCompletion();

    /**
     * @brief Handles completed subsequence by returning to the parent sequence.
     * Pops completed sequence, transitions parent to NODE_EXECUTED.
     */
    void HandleSubsequenceCompletion();

    /**
     * @brief Activates fallback sequence when primary sequence fails.
     *
     * FAILURE RECOVERY:
     * 1. Increment fallback_attempt_count
     * 2. If >= MAX_FALLBACK_ATTEMPTS: halt character (set is_halted=true)
     * 3. Otherwise:
     *    - Clear interruption memories for failed sequence
     *    - Reset failed sequence to entry point
     *    - Pop failed sequence
     *    - Push random fallback sequence
     *
     * This prevents characters from getting stuck in unexecutable sequences.
     */
    void HandleSequenceFailure();

    /**
     * @brief Centralized runtime error handler.
     *
     * FAILURE HANDLING:
     * - Logs detailed error context (reason, IDs, additional info)
     * - Transitions current sequence to FAILED state
     * - Optionally stops processing (for terminal errors)
     *
     * Common failure reasons:
     * - NODE_NOT_FOUND: Invalid node_id reference
     * - ACTION_NOT_FOUND: Invalid action_id reference
     * - NO_VALID_ENTITIES: No entities satisfy action preconditions
     * - NO_VALID_TRANSITIONS: No transitions from current node
     * - PRECONDITIONS_FAILED: Action preconditions not satisfied
     *
     * @param context Structured failure information for logging
     */
    void HandleRuntimeFailure(const RuntimeFailureContext& context);

// ═══════════════════════════════════════════════════════════════════════════
// STATE ACCESSORS (for inspection/debugging)
// ═══════════════════════════════════════════════════════════════════════════
public:
    MemorySystem& GetMemorySystem() { return memory; }

    [[nodiscard]]
    int64_t GetCurrentActionToken() const { return current_action_token; }

    [[nodiscard]]
    int32_t GetCurrentActionTargetId() const { return current_action_target_id; }

    [[nodiscard]]
    int32_t GetCurrentActionId() const { return current_action_id; }

    [[nodiscard]]
    bool IsProcessing() const { return is_processing; }

    [[nodiscard]]
    const std::shared_ptr<Sequence>& GetMainSequence() const { return main_sequence; }

    [[nodiscard]]
    bool HasFallbackSequence(int32_t sequence_id) const;

    [[nodiscard]]
    std::shared_ptr<Sequence> GetFallbackSequenceById(int32_t sequence_id) const;

    [[nodiscard]]
    std::shared_ptr<Sequence> FindInterruptionHandler(int32_t interruption_id) const;
};
}
