# KingdomCore 독립 빌드 — UE 없이 초 단위 이터레이션용.
# UE 쪽 빌드는 KingdomCore.Build.cs가 같은 소스를 사용한다.
#
# 빌드 디렉터리는 반드시 Source/ 밖(Intermediate/CMake)에 둔다 — UBT가
# Source/KingdomCore 아래의 모든 소스를 긁어가므로 산출물이 섞이면 링크가 깨진다.
# 리포지토리 루트에서:
#
#   cmake -S Source/KingdomCore -B Intermediate/CMake -G Ninja
#   cmake --build Intermediate/CMake
#   ctest --test-dir Intermediate/CMake --output-on-failure

cmake_minimum_required(VERSION 3.20)
project(KingdomCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # /utf-8: 소스가 UTF-8(BOM 없음)이므로 CP949 환경에서도 경고 없이 빌드
    add_compile_options(/utf-8)
endif()

add_library(KingdomCore STATIC
    Private/Archive.cpp
    Private/RandomStream.cpp
    Private/Simulation.cpp
    Private/Version.cpp
)
target_include_directories(KingdomCore PUBLIC Public)

if(MSVC)
    target_compile_options(KingdomCore PRIVATE /W4)
else()
    target_compile_options(KingdomCore PRIVATE -Wall -Wextra)
endif()

enable_testing()

# 테스트는 UBT가 모듈 소스로 긁어가지 않도록 모듈 밖(Source/KingdomCoreTests)에 둔다
add_executable(KingdomCoreTests
    ${CMAKE_CURRENT_SOURCE_DIR}/../KingdomCoreTests/TestMain.cpp
)
target_include_directories(KingdomCoreTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../KingdomCoreTests)
target_link_libraries(KingdomCoreTests PRIVATE KingdomCore)
add_test(NAME KingdomCoreTests COMMAND KingdomCoreTests)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT KingdomCoreTests)
