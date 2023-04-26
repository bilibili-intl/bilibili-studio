#ifndef BILILIVE_PUBLIC_COMMON_REFCOUNTED_DICTIONARY_H
#define BILILIVE_PUBLIC_COMMON_REFCOUNTED_DICTIONARY_H


#include "base\memory\ref_counted.h"
#include "base\values.h"
#include "base\callback.h"
#include "base\memory\scoped_ptr.h"

class RefCountedDictionary
    : public base::RefCountedThreadSafe<RefCountedDictionary>
{
public:
    virtual base::DictionaryValue *value()
    {
        return value_.get();
    }

    static RefCountedDictionary *TakeDictionary(base::DictionaryValue *value);

    static base::DictionaryValue *RecursiveDeepCopy(base::DictionaryValue *value);

private:
    scoped_ptr<base::DictionaryValue> value_;
};

typedef scoped_refptr<RefCountedDictionary> RefDictionary;
typedef base::Callback<void(RefDictionary,const std::string& json_content)> RefDictionaryParamClosure;

#endif