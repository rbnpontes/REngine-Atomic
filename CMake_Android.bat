@echo off
set src_dir=%cd%
set toolchain_path=%cd%\build\cmake\android.toolchain.cmake
set make_path=%cd%\prebuilt\windows-x86_64\bin\make.exe

cmake -E make_directory "../REngine-Android"
cmake -S "%src_dir%" -B "../REngine-Android" -G "Unix Makefiles" -DCMAKE_TOOLCHAIN=%src_dir%/Build/CMake/Toolchains/android.toolchain.cmake ^
    -DCMAKE_MAKE_PROGRAM=%make_path%
    -DANDROID_PLATFORM=android-26 -DANDROID_STL=c++_static ^
    -DATOMIC_EDITOR=OFF -DATOMIC_PROFILING=OFF -DATOMIC_JAVASCRIPT=OFF -DATOMIC_DOTNET=OFF
