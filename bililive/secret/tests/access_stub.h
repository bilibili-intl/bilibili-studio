/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_TESTS_ACCESS_STUB_H_
#define BILILIVE_SECRET_TESTS_ACCESS_STUB_H_

#include "bililive/public/secret/bililive_secret.h"

void SetSecretHandle(BililiveSecret* secret);

BililiveSecret* GetSecretHandle();

void ClearSecretHandle();

#endif  // BILILIVE_SECRET_TESTS_ACCESS_STUB_H_
