cd /d "%~dp0"
call :do_build Win32 Debug
call :do_build x64 Debug
call :do_build Win32 Release
call :do_build x64 Release
exit /b

:do_build
Setlocal
set plat=%1
if "%1"=="Win32" set plat=x86
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" %plat% -vcvars_ver=14.16
echo y | rd /s build
cmake -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=%2 -DCMAKE_INSTALL_PREFIX="%~dp0\dist\%1\%2"
cmake --build build --config %2
cmake --install build --config %2
echo y | rd /s build
EndLocal
exit /b
