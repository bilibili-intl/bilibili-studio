Getting Started
===

## Build Instructions

Operating system：Windows 7+，recommend Win10
IDE：[Visual Studio 2019 latest 16.11.19 SDK version 10.0.19041.0](https://docs.microsoft.com/zh-CN/visualstudio/releases/2019/system-requirements)
SSD：recommend

1. Download the project code locally，and switch to git **main** branch;
2. Install the latest version of cmake and add the path to the system environment path;
3. Open `bilibili-studio\bililive\bililive-resources.sln`，Compile the entire solution;（Each time you modify the resource, please **Rebuild** the whole resource solution）
4. Open `bilibili-studio\bililive\bililive.sln`，compile `bililive`，Dependent projects are compiled automatically;
5. Have a cup of coffee.

## Wiki Road Map


## Cmake（3.20.0 or later）

Currently cmake is only used for cmake project generation for obs related projects，other projects do not use cmake

## Package

1. First into the build\make_installer directory, then copy the program binaries and related configuration、resource data to the program_raw directory;
2. Open the innopackage.py file, modify the relevant parameters in the test() function, such as version、platform and other information to save;
3. Then open cmd command line and run: python innopackage.py or just run innopackage.py file to package;
4. After successfully packaged, the information "Successful compile" is displayed, and the installation package is generated In the current directory.