#pragma once

#include "bililive/common/bililive_context.h"
#include "bililive/secret/net/request_connection_common_types.h"
#include "bililive/secret/services/service_utils.h"

namespace {

    using secret::RequestParams;
    using secret::RequestParams2;

    RequestParams CommonRequestParams() {
        using namespace secret;

        return RequestParams{
            { kArgTimestamp, GetUnixTimestamp() },
            { kArgAppVersion, BililiveContext::Current()->GetExecutableVersionAsASCII() },
            { kArgAppKey, kAppKey },
            { kArgPlatform, kPlatform },
            { "build", std::to_string(BililiveContext::Current()->GetExecutableBuildNumber()) },
            { "access_key", QueryAccessToken() },
        };
    }

    RequestParams2 CommonRequestParams2() {
        using namespace secret;

        return RequestParams2{
            { kArgTimestamp, GetUnixTimestamp() },
            { kArgAppVersion, BililiveContext::Current()->GetExecutableVersionAsASCII() },
            { kArgAppKey, kAppKey },
            { kArgPlatform, kPlatform },
            { "build", std::to_string(BililiveContext::Current()->GetExecutableBuildNumber()) },
            { "access_key", QueryAccessToken() },
        };
    }

}