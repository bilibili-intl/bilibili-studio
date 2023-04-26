#pragma once

#include <windef.h>

#include <functional>
#include <vector>

namespace cef_proxy 
{
    struct MouseEvent 
    {
        int x;
        int y;
        uint32_t modifiers;
    };

    enum class MouseButtonType 
    {
        MBT_LEFT = 0,
        MBT_MIDDLE,
        MBT_RIGHT,
    };

    enum class KeyEventTpye 
    {
        KEYEVENT_RAWKEYDOWN = 0,
        KEYEVENT_KEYDOWN,
        KEYEVENT_KEYUP,
        KEYEVENT_CHAR
    } ;

    struct KeyEvent 
    {
        KeyEventTpye type;
        uint32_t modifiers;
        int windows_key_code;
        int native_key_code;
        int is_system_key;
        wchar_t character;
        wchar_t unmodified_character;
        int focus_on_editable_field;
    };
}



