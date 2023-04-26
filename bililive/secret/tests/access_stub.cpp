/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/tests/access_stub.h"

#include "base/logging.h"

namespace {

BililiveSecret* secret_handle = nullptr;

}   // namespace

void SetSecretHandle(BililiveSecret* secret)
{
    DCHECK(secret_handle == nullptr);
    secret_handle = secret;
}

BililiveSecret* GetSecretHandle()
{
    DCHECK(secret_handle != nullptr);
    return secret_handle;
}

void ClearSecretHandle()
{
    secret_handle = nullptr;
}
