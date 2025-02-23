cmake_minimum_required(VERSION 3.29)

project(edflib LANGUAGES C CXX)

set(CMAKE_C_STANDARD 89)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(edflib SHARED 
src/edflib.cc
src/edflib_api.cc
)

target_include_directories(edflib PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_options(edflib PRIVATE /W4 /EHsc)
else()
    target_compile_options(edflib PRIVATE -Wall -Wextra -pedantic)
endif()


target_compile_definitions(edflib PRIVATE EDFLIB_EXPORTS)



# 添加C#测试
if(WIN32)
    find_program(DOTNET_EXE "dotnet" PATHS ENV ProgramFiles)
    if(NOT DOTNET_EXE)
        message(FATAL_ERROR "dotnet CLI not found!")
    endif()

    # 配置C#测试项目
    add_custom_command(
        TARGET edflib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/csharp/bin/${CMAKE_BUILD_TYPE}/
        COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_FILE:edflib> 
            ${CMAKE_CURRENT_SOURCE_DIR}/csharp/bin/${CMAKE_BUILD_TYPE}/
        COMMAND ${DOTNET_EXE} build 
            ${CMAKE_CURRENT_SOURCE_DIR}/csharp
            --configuration ${CMAKE_BUILD_TYPE}
        COMMAND ${DOTNET_EXE} run 
            --project ${CMAKE_CURRENT_SOURCE_DIR}/csharp
            --no-build
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building and running C# tests..."
    )
endif()
