#pragma once
#include <stdexcept>
#include <string>
#include <cstdint>

namespace AmbientCharacterBehavior {
/**
 * @brief Abstract base class for all memory types in the behavior framework.
 *
 * Provides common functionality for tracking when decisions were made (timestamp) and comparing memory recency.
 * Each derived class implements type specific matching criteria to determine what constitutes as "the same decision".
 *
 * Design rationale: Inheritance allows shared time tracking logic while enabling polymorphic matching through virtual
 * MatchesMemory(). This supports the exploration-exploitation algorithm which needs to:
 * 1. Check if a decision was made before (matching)
 * 2. Determine which decision is older (comparison)
 *
 */
class BaseMemory
{
protected:
    /**
     * @brief Timestamp of when the decision was made (milliseconds since simulation start)
     *
     * Used for recency comparison in least recently used selection. Protected to allow derived classes to access directly.
     */
    int64_t creation_time;
public:
    /**
     * @brief Construct a memory with a creation timestamp
     * @param creation_time Timestamp in milliseconds (must be non-negative)
     * @throws std::invalid_argument if time < 0
     */
    explicit BaseMemory(const int64_t creation_time) : creation_time(creation_time) {
        if(creation_time < 0)
        {
            throw std::invalid_argument("BaseMemory: time cannot be negative, got " + std::to_string(creation_time));
        }
    }

    virtual ~BaseMemory() = default;

    /**
     * @brief Check if this memory represents the same decision as another
     *
     * Each memory will define its own matching criteria.
     *
     * @param other Memory to compare against
     * @return true if memories represent the same decision
     */
    [[nodiscard]]
    virtual bool MatchesMemory(const BaseMemory& other) const = 0;

    [[nodiscard]]
    int64_t GetCreationTime() const { return creation_time; }

    /**
     * @brief Compare the recency of two memories
     * @param other Memory to compare against
     * @return true if this memory is older (smaller timestamp)
     */
    virtual bool IsOlderThan(BaseMemory& other) const {
        return creation_time < other.GetCreationTime();
    }
};

}