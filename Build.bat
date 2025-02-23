@echo off
setlocal

set "VS_PATH=C:\\Program Files\\Microsoft Visual Studio\\2022\\Community"

call "%VS_PATH%\\VC\\Auxiliary\\Build\\vcvars64.bat"  -vcvars_ver=14.2

set "WORKSPACE=%~dp0"

cmake -B "%WORKSPACE%\\build" ^
    -DCMAKE_CXX_COMPILER=cl ^
    -DCMAKE_C_COMPILER=cl ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -G Ninja ^
    -S "%WORKSPACE%"

cd "%WORKSPACE%\\build"
cmake --build . --verbose
cd "%WORKSPACE%"

endlocal