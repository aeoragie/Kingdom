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

# 소스는 폴더째 모은다(UBT와 같은 규칙: 폴더 = 모듈). 파일을 더해도 이 목록은 고치지 않는다.
# CONFIGURE_DEPENDS: 빌드할 때마다 파일 목록이 바뀌었는지 보고, 바뀌었으면 CMake가 다시 구성한다.
# 헤더는 빌드에 필요 없지만 VS 솔루션 탐색기에 보이도록 함께 넣는다.
file(GLOB_RECURSE KingdomCoreSources CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Private/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Private/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Public/*.h
)
add_library(KingdomCore STATIC ${KingdomCoreSources})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${KingdomCoreSources})
target_include_directories(KingdomCore PUBLIC Public)

if(MSVC)
    target_compile_options(KingdomCore PRIVATE /W4)
else()
    target_compile_options(KingdomCore PRIVATE -Wall -Wextra)
endif()

enable_testing()

# 테스트는 UBT가 모듈 소스로 긁어가지 않도록 모듈 밖(Source/KingdomCoreTests)에 둔다
file(GLOB_RECURSE KingdomCoreTestSources CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../KingdomCoreTests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../KingdomCoreTests/*.h
)
add_executable(KingdomCoreTests ${KingdomCoreTestSources})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../KingdomCoreTests FILES ${KingdomCoreTestSources})
target_include_directories(KingdomCoreTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../KingdomCoreTests)
target_link_libraries(KingdomCoreTests PRIVATE KingdomCore)
# 테스트가 실제 규칙 데이터(저장소 루트의 Data/)를 읽는다
target_compile_definitions(KingdomCoreTests PRIVATE KINGDOM_DATA_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/../../Data")

# Data/ 파일을 VS 솔루션 탐색기에 보이게 한다(컴파일하지 않는다). 테스트가 실행할 때 읽으므로
# CSV만 고쳤으면 다시 빌드하지 않고 테스트만 다시 돌리면 된다.
file(GLOB_RECURSE KingdomDataFiles CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Data/*.csv
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Data/*.md
)
target_sources(KingdomCoreTests PRIVATE ${KingdomDataFiles})
set_source_files_properties(${KingdomDataFiles} PROPERTIES HEADER_FILE_ONLY ON)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../.. FILES ${KingdomDataFiles})
add_test(NAME KingdomCoreTests COMMAND KingdomCoreTests)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT KingdomCoreTests)