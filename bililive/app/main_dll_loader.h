#ifndef BILILIVE_APP_MAIN_DLL_LOADER_H_
#define BILILIVE_APP_MAIN_DLL_LOADER_H_

#include <windows.h>

#include "base/files/file_path.h"
#include "base/strings/string16.h"

class MainDllLoader {
public:
    MainDllLoader();

    virtual ~MainDllLoader();

    int Launch(HINSTANCE instance);

protected:
    virtual string16 GetRegistryPath() = 0;

    virtual void OnBeforeLaunch(const base::FilePath& main_dir);

    virtual int OnBeforeExit(int return_code, const base::FilePath& main_dir);

    HMODULE Load(base::FilePath* module_dir);

private:
    HMODULE dll_;
};

MainDllLoader* MakeMainDllLoader();

#endif  // BILILIVE_APP_MAIN_DLL_LOADER_H_