#include "bilibase/error_exception_util.h"

namespace bilibase {

void Guarantor::Require() const
{
    Raise();
}

void Guarantor::Require(const std::string& msg)
{
    exception_desc_ << "Extra Message: " << msg << "\n";
    Raise();
}

void Guarantor::Raise() const
{
    throw std::runtime_error(exception_desc_.str());
}

}   // namespace bilibase