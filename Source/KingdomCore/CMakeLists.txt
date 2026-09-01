# KingdomCore 독립 빌드 — UE 없이 초 단위 이터레이션용.
# UE 쪽 빌드는 KingdomCore.Build.cs가 같은 소스를 사용한다.
#
#   cmake -S . -B build -G Ninja
#   cmake --build build
#   ctest --test-dir build --output-on-failure

cmake_minimum_required(VERSION 3.20)
project(KingdomCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # /utf-8: 소스가 UTF-8(BOM 없음)이므로 CP949 환경에서도 경고 없이 빌드
    add_compile_options(/utf-8)
endif()

add_library(KingdomCore STATIC
    Private/Version.cpp
)
target_include_directories(KingdomCore PUBLIC Public)

if(MSVC)
    target_compile_options(KingdomCore PRIVATE /W4)
else()
    target_compile_options(KingdomCore PRIVATE -Wall -Wextra)
endif()

enable_testing()

add_executable(KingdomCoreTests
    Tests/TestMain.cpp
)
target_link_libraries(KingdomCoreTests PRIVATE KingdomCore)
add_test(NAME KingdomCoreTests COMMAND KingdomCoreTests)
