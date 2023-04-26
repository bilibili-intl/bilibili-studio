CD %~dp0openssl-1.0.2h

:set env="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\vcvars32.bat"
set env="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars32.bat"
call %env%
set VisualStudioVersion=15.0

call nmake -f ms\ntdll.mak install

set build_lib_dir="build\lib\"
set dep_lib_dir="..\lib\"
copy /Y %build_lib_dir%\libeay32.lib %dep_lib_dir%
copy /Y %build_lib_dir%\ssleay32.lib %dep_lib_dir%

set build_bin_dir="build\bin\"
set dep_bin_dir="..\bin\"
copy /Y %build_bin_dir%\libeay32.dll %dep_bin_dir%
copy /Y %build_bin_dir%\ssleay32.dll %dep_bin_dir%
