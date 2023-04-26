#ifndef BILIBASE_ERROR_EXCEPTION_UTIL_H_
#define BILIBASE_ERROR_EXCEPTION_UTIL_H_

#include <sstream>
#include <string>

#include "bilibase/basic_macros.h"
#include "bilibase/string_encoding_conversions.h"

namespace bilibase {

inline bool NotReached()
{
    return false;
}

#define GUARANTOR_A(x) GUARANTOR_OP(x, B)
#define GUARANTOR_B(x) GUARANTOR_OP(x, A)
#define GUARANTOR_OP(x, next) \
    GUARANTOR_A.CaptureValue(#x, (x)).GUARANTOR_##next

#define MAKE_GUARANTOR(cond) \
    bilibase::Guarantor(cond, __FILE__, __LINE__)

#define ENSURE(cond)                                                                \
    static_assert(std::is_same<std::remove_const_t<decltype(cond)>, bool>::value,   \
                  "cond must be a bool expression");                                \
    (cond) ? (void)0 : MAKE_GUARANTOR(#cond).GUARANTOR_A

class Guarantor {
public:
    Guarantor(const char* msg, const char* file_name, int line)
    {
        exception_desc_ << "Failed: " << msg
                        << "\nFile: " << file_name << " Line: " << line
                        << "\nChecked Variables:\n";
    }

    ~Guarantor() = default;

    DISABLE_COPY(Guarantor);

    // Capture diagnostic variables.

    template<typename T>
    Guarantor& CaptureValue(const char* name, T&& value)
    {
        exception_desc_ << "    " << name << " = " << value << "\n";
        return *this;
    }

    Guarantor& CaptureValue(const char* name, const std::wstring& value)
    {
        std::string converted = WideToUTF8(value);
        return CaptureValue(name, converted);
    }

    Guarantor& CaptureValue(const char* name, const wchar_t* value)
    {
        std::string converted = WideToUTF8(value);
        return CaptureValue(name, converted);
    }

    void Require() const;

    void Require(const std::string& msg);

    // Added to throw a specific exception that you know how to handle it when the
    // condition is violated.
    template<typename E>
    void Require()
    {
        static_assert(std::is_base_of<std::exception, E>::value,
                      "E must be a subclass of std::exception");
        throw E(exception_desc_.str());
    }

    // Accessory stubs for infinite variable capture.
    Guarantor& GUARANTOR_A = *this;
    Guarantor& GUARANTOR_B = *this;

private:
    void Raise() const;

private:
    std::ostringstream exception_desc_;
};

}   // namespace bilibase

#endif  // BILIBASE_ERROR_EXCEPTION_UTIL_H_