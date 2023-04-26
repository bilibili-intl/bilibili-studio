#ifndef BILILIVE_BILILIVE_PREFS_SCOPED_USER_PREF_UPDATE_H
#define BILILIVE_BILILIVE_PREFS_SCOPED_USER_PREF_UPDATE_H

#include <string>

#include "base/basictypes.h"
#include "base/prefs/pref_service.h"
#include "base/threading/non_thread_safe.h"
#include "base/values.h"

class PrefService;

namespace base
{
    class DictionaryValue;
    class ListValue;
}

namespace subtle
{
    class ScopedUserPrefUpdateBase
        : public base::NonThreadSafe
    {
    protected:
        ScopedUserPrefUpdateBase(PrefService *service, const char *path);

        ~ScopedUserPrefUpdateBase();

        base::Value *GetValueOfType(base::Value::Type type);

    private:
        void Notify();

        PrefService *service_;
        std::string path_;
        base::Value *value_;

        DISALLOW_COPY_AND_ASSIGN(ScopedUserPrefUpdateBase);
    };
}

template <typename T, base::Value::Type type_enum_value>
class ScopedUserPrefUpdate
    : public subtle::ScopedUserPrefUpdateBase
{
public:
    ScopedUserPrefUpdate(PrefService *service, const char *path)
        : ScopedUserPrefUpdateBase(service, path) {}

    virtual ~ScopedUserPrefUpdate() {}

    virtual T *Get()
    {
        return static_cast<T *>(GetValueOfType(type_enum_value));
    }

    T &operator*()
    {
        return *Get();
    }

    T *operator->()
    {
        return Get();
    }

private:
    DISALLOW_COPY_AND_ASSIGN(ScopedUserPrefUpdate);
};

typedef ScopedUserPrefUpdate<base::DictionaryValue,
        base::Value::TYPE_DICTIONARY>
        DictionaryPrefUpdate;
typedef ScopedUserPrefUpdate<base::ListValue, base::Value::TYPE_LIST>
ListPrefUpdate;

#endif