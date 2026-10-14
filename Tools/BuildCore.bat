@echo off
chcp 65001 >nul
rem KingdomCore를 UE 없이 CMake로 빌드하고 테스트를 돌린다.
rem VS 개발자 환경(cl, cmake, ninja)을 직접 불러오므로 일반 명령 프롬프트에서 실행해도 된다.
setlocal
call "%~dp0Environment.bat" || exit /b 1

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -property installationPath`) do set "VS_ROOT=%%I"
if not defined VS_ROOT (
	echo [Kingdom] Visual Studio를 찾지 못했습니다.
	if "%~1"=="" pause
	exit /b 1
)
call "%VS_ROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul

cd /d "%REPO_ROOT%"
cmake -S Source/KingdomCore -B Intermediate/CMake -G Ninja -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 goto :Failed
cmake --build Intermediate/CMake
if errorlevel 1 goto :Failed
"Intermediate\CMake\KingdomCoreTests.exe"
if errorlevel 1 goto :Failed

echo [Kingdom] 코어 빌드와 테스트 통과.
if "%~1"=="" pause
exit /b 0

:Failed
echo [Kingdom] 실패.
if "%~1"=="" pause
exit /b 1
