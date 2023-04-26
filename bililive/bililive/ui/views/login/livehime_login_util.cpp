#include "livehime_login_util.h"

#include <memory>

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"

#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/common/bililive_constants.h"

namespace livehime
{
    std::string GetAvatarBinaryDataByMid(int64 mid)
    {
        std::string content;
        base::FilePath avatar_path;
        if (PathService::Get(bililive::DIR_USER_DATA, &avatar_path))
        {
            avatar_path = avatar_path.Append(std::to_wstring(mid)).Append(bililive::kUserFaceFileName);
            base::ThreadRestrictions::ScopedAllowIO allow_io;
            if (base::PathExists(avatar_path))
            {
                file_util::ReadFileToString(avatar_path, &content);
            }
        }
        return content;
    }

    std::string CefCookiesToString(const cef_proxy::cookies& cookies)
    {
        base::ListValue root;
        for (auto& cookie : cookies)
        {
            base::DictionaryValue* item = new base::DictionaryValue();

            item->SetString("name", cookie.name);
            item->SetString("value", cookie.value);
            item->SetString("domain", cookie.domain);
            item->SetString("path", cookie.path);
            item->SetInteger("secure", cookie.secure);
            item->SetInteger("httponly", cookie.httponly);
            item->SetInteger64("creation", cookie.creation);
            item->SetInteger64("last_access", cookie.last_access);
            item->SetInteger("has_expires", cookie.has_expires);
            item->SetInteger64("expires", cookie.expires);

            root.Append(item);
        }
        std::string json_str;
        base::JSONWriter::Write(&root, &json_str);
        return json_str;
    }

    cef_proxy::cookies StringToCefCookies(const std::string& str)
    {
        cef_proxy::cookies cookies;
        std::unique_ptr<base::Value> values(base::JSONReader::Read(str));
        const base::ListValue* list_value = nullptr;
        if (!values || !values->GetAsList(&list_value)) {
            return cookies;
        }

        for (auto iter : *list_value)
        {
            const base::DictionaryValue* dict_value = nullptr;
            if (!iter->GetAsDictionary(&dict_value)) {
                NOTREACHED();
                continue;
            }

            base::string16 wstr;
            cef_proxy::cookie_t cookie;
            dict_value->GetString("name", &wstr);
            cookie.name = base::UTF16ToUTF8(wstr);

            dict_value->GetString("value", &wstr);
            cookie.value = base::UTF16ToUTF8(wstr);

            dict_value->GetString("domain", &wstr);
            cookie.domain = base::UTF16ToUTF8(wstr);

            dict_value->GetString("path", &wstr);
            cookie.path = base::UTF16ToUTF8(wstr);

            dict_value->GetInteger("secure", &cookie.secure);
            dict_value->GetInteger("httponly", &cookie.httponly);
            dict_value->GetInteger64("creation", &cookie.creation);
            dict_value->GetInteger64("last_access", &cookie.last_access);
            dict_value->GetInteger("has_expires", &cookie.has_expires);
            dict_value->GetInteger64("expires", &cookie.expires);

            cookies.push_back(cookie);
        }
        return cookies;
    }

}