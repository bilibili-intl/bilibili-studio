#ifndef BILIBASE_WIN_WIN10_VER_H_
#define BILIBASE_WIN_WIN10_VER_H_


namespace bilibase {
namespace win {

    bool IsWindows10V2004OrGreater();
    bool IsWindowsVersionOrGreater(DWORD build);
}
}

#endif  // BILIBASE_WIN_WIN10_VER_H_