#ifndef BILILIVE_COMMON_BILILIVE_RESULT_CODES_H_
#define BILILIVE_COMMON_BILILIVE_RESULT_CODES_H_

namespace bililive {

enum ResultCode : int {
    ResultCodeNormalExit = 0,

    ResultCodeSignOut,

    ResultCodeLoginCancelled,

    ResultCodeErrorOccurred,

    ResultCodeRestart
};

}   // namespace bililive

#endif  // BILILIVE_COMMON_BILILIVE_RESULT_CODES_H_
