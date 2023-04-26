#include "refcounted_dictionary.h"
#include "base\logging.h"
#include "base\values.h"

namespace
{
    base::Value *RecursiveCopy(const base::Value *node)
    {
        DCHECK(node);
        switch (node->GetType())
        {
        case base::Value::TYPE_LIST:
        {
            const base::ListValue *list = static_cast<const base::ListValue *>(node);
            base::ListValue *copy = new base::ListValue;
            for (base::ListValue::const_iterator it = list->begin(); it != list->end();
                 ++it)
            {
                base::Value *child_copy = RecursiveCopy(*it);
                if (child_copy)
                {
                    copy->Append(child_copy);
                }
            }
            return copy;
        }

        case Value::TYPE_DICTIONARY:
        {
            const DictionaryValue *dict = static_cast<const DictionaryValue *>(node);
            DictionaryValue *copy = new DictionaryValue;
            for (DictionaryValue::Iterator it(*dict); !it.IsAtEnd(); it.Advance())
            {
                Value *child_copy = RecursiveCopy(&it.value());
                if (child_copy)
                {
                    copy->SetWithoutPathExpansion(it.key(), child_copy);
                }
            }

            return copy;
        }

        case base::Value::TYPE_NULL:
        {
            return base::Value::CreateNullValue();
        }

        case base::Value::TYPE_INTEGER:
        {
            int64_t int_value;
            if (node->GetAsInteger64(&int_value))
            {
                return base::Value::CreateIntegerValue(int_value);
            }
            return NULL;
        }

        case base::Value::TYPE_BOOLEAN:
        {
            bool bool_value;
            if (node->GetAsBoolean(&bool_value))
            {
                return base::Value::CreateBooleanValue(bool_value);
            }
            return NULL;
        }

        case base::Value::TYPE_DOUBLE:
        {
            double double_value;
            if (node->GetAsDouble(&double_value))
            {
                return base::Value::CreateDoubleValue(double_value);
            }
            return NULL;
        }

        case base::Value::TYPE_STRING:
        {
            const base::StringValue *string_value = static_cast<const base::StringValue *>(node);
            return base::Value::CreateStringValue(string_value->value());
        }

        case base::Value::TYPE_BINARY:
        {
            const base::BinaryValue *binary_value = static_cast<const base::BinaryValue *>(node);
            return base::BinaryValue::CreateWithCopiedBuffer(binary_value->GetBuffer(), binary_value->GetSize());
        }

        default:
            return NULL;
        }
    }
}

RefCountedDictionary *RefCountedDictionary::TakeDictionary(base::DictionaryValue *value)
{
    RefCountedDictionary *self = new RefCountedDictionary;
    self->value_.reset(value->DeepCopy());
    return self;
}

base::DictionaryValue *RefCountedDictionary::RecursiveDeepCopy(base::DictionaryValue *value)
{
    DCHECK(value->IsType(base::Value::TYPE_DICTIONARY));
    base::Value *result_value = RecursiveCopy(value);
    base::DictionaryValue *dict_value;
    if (result_value->GetAsDictionary(&dict_value))
    {
        return dict_value;
    }
    return NULL;
}