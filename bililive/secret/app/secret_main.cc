#include "bililive/secret/core/bililive_secret_core_impl.h"

#define DLLEXPORT __declspec(dllexport)

extern "C" DLLEXPORT BililiveSecret* __cdecl CreateBililiveSecret();

BililiveSecret* __cdecl CreateBililiveSecret()
{
    return new BililiveSecretCoreImpl();
}
