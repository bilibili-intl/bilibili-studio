// Copyright (c) 2016 ucclkp <ucclkp@gmail.com>.
// This file is part of utils project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef UTILS_PLATFORM_UTILS_H_
#define UTILS_PLATFORM_UTILS_H_

// 指定大小端
//#define ENDIAN_BE
#define ENDIAN_LE

// 指定操作系统
#if defined(_WIN32)
#define OS_WINDOWS
#elif defined(__APPLE__)
#define OS_MAC
#else
#define OS_UNKNOWN
#endif

// 入口点

#ifdef OS_WINDOWS
#include <Windows.h>
#define GUI_MAIN()  \
    int APIENTRY wWinMain(  \
        HINSTANCE hInstance, HINSTANCE hPrevInstance,  \
        LPWSTR lpCmdLine, int nShowCmd)
#else
#define GUI_MAIN()  \
    int main()
#endif

#endif  // UTILS_PLATFORM_UTILS_H_