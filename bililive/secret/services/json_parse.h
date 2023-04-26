#ifndef BILILIVE_SECRET_SERVICE_JSON_PARSE_H_
#define BILILIVE_SECRET_SERVICE_JSON_PARSE_H_

#include "base/json/json_reader.h"
#include "base/values.h"
#include "base/strings/string_number_conversions.h"

#include <set>

#include "bililive/secret/services/service_utils.h"

using secret::ResponseInfo;

extern const std::set<std::string> log_interface_blacklist;

#define BEGIN_CHECK_DICTIONARY_FIELD_PARSE(interface_name)    \
    {                                                         \
        bool anyone_parse_faild = false;                      \
        const char* const web_api_name = interface_name;

#define JC_LOG_FAILED(fieldname)    \
    anyone_parse_faild = true;      \
    LOG(WARNING) << "Parse " << web_api_name << " response field failure: " << fieldname;

#define CHECK_DICTIONARY_FIELD_PARSE(func, fieldname, out_value)                         \
    if (!func(fieldname, out_value))                                                      \
    {                                                                                    \
        JC_LOG_FAILED(fieldname)  \
    }

#define RET_IF_FIELD_PARSE_FAILED(func, fieldname, out_value)  \
    if (!func(fieldname, out_value)) {                                        \
        LOG(WARNING) << "Parse " << web_api_name << " response field failure: " << fieldname  \
                     << "\n" << "received data: " << data;  \
        return {};                                              \
    }

#define END_CHECK_DICTIONARY_FIELD_PARSE                      \
    }

namespace {

bool GetJsonValueByKey(
    const base::Value* base_value, const std::string& key, int* out_value) {
    if (base_value) {
        if (base_value->GetType() == base::Value::Type::TYPE_STRING) {
            std::string value;
            base_value->GetAsString(&value);
            base::StringToInt(value, out_value);
            return true;
        }

        if (base_value->GetType() == base::Value::Type::TYPE_INTEGER) {
            base_value->GetAsInteger(out_value);
            return true;
        }
    }

    return false;
}

bool GetJsonValueByKey(
    const base::Value* base_value, const std::string& key, int64_t* out_value) {
    if (base_value) {
        if (base_value->GetType() == base::Value::Type::TYPE_STRING) {
            std::string value;
            base_value->GetAsString(&value);
            base::StringToInt64(value, out_value);
            return true;
        }

        if (base_value->GetType() == base::Value::Type::TYPE_INTEGER) {
            base_value->GetAsInteger64(out_value);
            return true;
        }
    }

    return false;
}

bool GetJsonValueByKey(
    const base::Value* base_value, const std::string& key, std::string* out_value) {
    if (base_value) {
        if (base_value->GetType() == base::Value::Type::TYPE_STRING) {
            base_value->GetAsString(out_value);
            return true;
        }

        if (base_value->GetType() == base::Value::Type::TYPE_INTEGER) {
            int64_t value = 0;
            base_value->GetAsInteger64(&value);
            (*out_value) = std::to_string(value);
            return true;
        }
    }

    return false;
}

bool GetJsonValueByKey(
    const base::DictionaryValue* dict_value, const std::string& key, std::string* out_value) {
    const base::Value* base_value = nullptr;
    dict_value->Get(key, &base_value);
    return GetJsonValueByKey(base_value, key, out_value);
}

bool GetJsonValueByKey(
    const base::DictionaryValue* dict_value, const std::string& key, int* out_value) {
    const base::Value* base_value = nullptr;
    dict_value->Get(key, &base_value);
    return GetJsonValueByKey(base_value, key, out_value);
}

bool GetJsonValueByKey(
    const base::DictionaryValue* dict_value, const std::string& key, int64_t* out_value) {
    const base::Value* base_value = nullptr;
    dict_value->Get(key, &base_value);
    return GetJsonValueByKey(base_value, key, out_value);
}

base::DictionaryValue* CommonParse(ResponseInfo info,
    const std::string& data,
    const std::string& interface_name,
    int* code) {
    if (info.response_code != 200 || data.empty()) {
        LOG(WARNING) << "Parse response failure: invalid status, or empty data!\n"
            << "interface_name:" << interface_name << ", "
            << "response info: " << info;
        return nullptr;
    }

    const base::DictionaryValue* dict_value = nullptr;
    std::unique_ptr<base::Value> values(base::JSONReader::Read(data));
    if (!values || !values->GetAsDictionary(&dict_value)) {
        LOG(WARNING) << "Parse response failure: not in json format!\n"
            << "interface_name:" <<interface_name << ", "
            << "received data: " << data;
        return nullptr;
    }

    if (!GetJsonValueByKey(dict_value, "code", code)) {
        LOG(WARNING) << "Parse response failure: missing `code` field!\n"
            << "interface_name:" << interface_name << ", "
            << "received data: " << data;
        return nullptr;
    }

    if (*code != 0)
    {
        // 不等于0不代表接口就有问题，但不好区分具体接口，所以所有接口code不为0都打出来
        LOG(WARNING) << "Parse response `code` not equal 0,\n"
            << "interface_name:" << interface_name << ", "
            << "received data: " << data;
    }

    base::DictionaryValue* content = dict_value->DeepCopy();

    if (log_interface_blacklist.count(interface_name) == 0) {
        LOG(INFO) << "Request interface succeeded!" << " IN: " << interface_name;
    }

    return content;
}

}

#endif  // BILILIVE_SECRET_SERVICE_JSON_PARSE_H_