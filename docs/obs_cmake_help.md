1、cmake project generation from the command line
   cmake.exe -G "Visual Studio 16 2019" -A Win32 -DCMAKE_CONFIGURATION_TYPES:STRING="Debug" -DVC_INCLUDEPATH:STRING="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.28.29333\include;;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.28.29333\atlmfc\include;;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\VS\include;;C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt;;;C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\winrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\cppwinrt;C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8\Include\um;" -DVC_LIBRARYPATH:STRING="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.28.29333\lib\x86;;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.28.29333\atlmfc\lib\x86;;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\VS\lib\x86;;C:\Program Files (x86)\Windows Kits\10\lib\10.0.19041.0\ucrt\x86;;;C:\Program Files (x86)\Windows Kits\10\lib\10.0.19041.0\um\x86;C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8\lib\um\x86;" -S "F:\ProjectCode\bililive-win\bililive\.." -B "F:\ProjectCode\bililive-win\bililive\..\out\livehime\."
   cmake.exe -G "Visual Studio 15 2017 Win32"  -DCMAKE_CONFIGURATION_TYPES:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="F:\ProjectCode\livehime\out\install\x86" -S "F:\ProjectCode\livehime" -B "F:\tmp" -T "Visual Studio 2017 (v141)"

2、Build the cmake project from the command line
   e.g., "cmake.exe --build "F:\ProjectCode\bililive-win\bililive\..\out\livehime\." --config Debug -j 8"，Builds the "ALL_BUILD.vcxproj" project in the specified directory
























