#ifndef BILILIVE_BILILIVE_COMMAND_UPDATER_DELEGATE_H_
#define BILILIVE_BILILIVE_COMMAND_UPDATER_DELEGATE_H_

// Do not declare a CommandParamsDetails directly, use either CommandParams or
// NoCommandParams().
class CommandParamsDetails {
public:
    CommandParamsDetails()
        : payload_ptr_(nullptr)
    {}

    CommandParamsDetails(const CommandParamsDetails &other)
        : payload_ptr_(other.payload_ptr_)
    {}

    ~CommandParamsDetails()
    {}

    uintptr_t map_key() const
    {
        return reinterpret_cast<uintptr_t>(payload_ptr_);
    }

    bool operator!=(const CommandParamsDetails &other) const
    {
        return payload_ptr_ != other.payload_ptr_;
    }

    bool operator==(const CommandParamsDetails &other) const
    {
        return payload_ptr_ == other.payload_ptr_;
    }

protected:
    explicit CommandParamsDetails(const void* ptr)
        : payload_ptr_(ptr)
    {}

    const void* payload_ptr_;
};

template <class T>
class CommandParams : public CommandParamsDetails
{
public:
    explicit CommandParams(T* ptr)
        : CommandParamsDetails(ptr)
    {}

    CommandParams(const CommandParamsDetails& other)
        : CommandParamsDetails(other)
    {}

    T* operator->() const
    {
        return ptr();
    }

    T* ptr() const
    {
        // The casts here allow this to compile with both T = Foo and T = const Foo.
        return static_cast<T*>(const_cast<void*>(payload_ptr_));
    }
};

inline CommandParams<void> EmptyCommandParams()
{
    return CommandParams<void>(nullptr);
}

inline CommandParams<void> DummyCommandParams()
{
    return CommandParams<void>(reinterpret_cast<void*>(1));
}

// Implement this interface so that your object can execute commands when needed.
class CommandUpdaterDelegate {
public:
    virtual void ExecuteCommandWithParams(int id, const CommandParamsDetails& params) = 0;

protected:
    virtual ~CommandUpdaterDelegate() {}
};

#endif  // BILILIVE_BILILIVE_COMMAND_UPDATER_DELEGATE_H_
