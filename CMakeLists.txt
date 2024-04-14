cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_ENABLE_EXPORTS FALSE)
project(fn2ptr C CXX)

find_package(Catch2 3.4 QUIET)
if(NOT Catch2_FOUND)
    Include(FetchContent)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v3.4.0 # or a later release
    )
    FetchContent_MakeAvailable(Catch2)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
# warning level 4 and all warnings as errors
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/W4>")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/WX>")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/wd4335>")
else()
    # lots of warnings and all warnings as errors
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wall>")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wno-missing-braces>")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wextra>")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Werror>")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-pedantic>")
    if (NOT WIN32)
        # Asan goes crazy on win32 clang
        add_compile_options("$<$<AND:$<CONFIG:DEBUG>,$<COMPILE_LANGUAGE:CXX>>:-fsanitize=address,undefined>")
        add_link_options("$<$<AND:$<CONFIG:DEBUG>,$<COMPILE_LANGUAGE:CXX>>:-fsanitize=address,undefined>")
        add_compile_options("$<$<AND:$<CONFIG:DEBUG>,$<COMPILE_LANGUAGE:CXX>>:-fsanitize-undefined-trap-on-error>")
    endif()
endif()

if (WIN32)
    add_compile_definitions("$<$<COMPILE_LANGUAGE:CXX>:_CRT_SECURE_NO_WARNINGS>")
    add_compile_definitions("$<$<COMPILE_LANGUAGE:CXX>:_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS>")
endif()

set(fn2ptr_includes
    include/fn2ptr.h
    include/fn_factory.h
    include/lambda_proxy.h
    include/fnptr_proxy.h
    include/mempage.h
    include/asm.h
)

if (WIN32)
    set(OS_DIR src/win)
elseif (UNIX)
    set(OS_DIR src/posix)
endif()
set(PLATFORM_DIR ${OS_DIR}/${CMAKE_SYSTEM_PROCESSOR})
message(STATUS OS=${OS_DIR}, PLATFORM=${PLATFORM_DIR})

add_library(fn2ptr
    ${fn2ptr_includes}
    ${OS_DIR}/mempage.cpp
    ${PLATFORM_DIR}/asm.cpp
)

target_include_directories(fn2ptr
    PRIVATE ${OS_DIR} ${PLATFORM_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_executable(fn2ptr_test
    tests/test.cpp
)

target_link_libraries(fn2ptr_test PRIVATE
    fn2ptr
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(fn2ptr_test)
