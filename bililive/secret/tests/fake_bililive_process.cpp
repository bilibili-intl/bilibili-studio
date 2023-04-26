/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/tests/fake_bililive_process.h"

namespace {

const char kLocale[] = "en-US";

}   // namespace

FakeBililiveProcess::FakeBililiveProcess()
    : first_run_(false),
      updated_run_(false),
      exit_as_logout_(false),
      secret_(nullptr)
{}

FakeBililiveProcess::~FakeBililiveProcess()
{}

const std::string& FakeBililiveProcess::GetApplicationLocale()
{
    return kLocale;
}

void FakeBililiveProcess::SetApplicationLocale(const std::string& locale)
{
    DCHECK(locale == kLocale);
}
