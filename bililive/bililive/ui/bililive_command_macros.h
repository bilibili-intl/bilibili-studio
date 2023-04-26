/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_MACROS_H_
#define BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_MACROS_H_

#include "base/logging.h"

#define BEGIN_BILILIVE_COMMAND_MAP(cmd_receiver, cmd_id, cmd_params)        \
    {                                                                       \
        auto __receiver = cmd_receiver;                                     \
        auto __id = cmd_id;                                                 \
        const auto& __params = cmd_params;                                  \
        switch (cmd_id) {                                                   \

#define ON_BILILIVE_COMMAND(cmd_id, fn)                                     \
    case cmd_id: {                                                          \
        fn(__receiver, __params);                                           \
    }                                                                       \
    break;                                                                  \

#define ON_BILILIVE_COMMAND_UNHANDLED_ERROR()                               \
    default: {                                                              \
        NOTREACHED() << "Unhandled command " << __id;                       \
    }                                                                       \
    break;                                                                  \

#define END_BILILIVE_COMMAND_MAP()                                          \
        }                                                                   \
    }

#endif  // BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_MACROS_H_
