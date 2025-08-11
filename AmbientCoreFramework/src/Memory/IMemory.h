class IMemory
{
public:
    virtual ~IMemory() = default;
    virtual bool MatchesMemory(const IMemory& other) const = 0;
    virtual int GetLastUsedTime() const = 0;
    virtual bool IsOlderThan(const IMemory& other) const = 0;
};