@echo off
rem 다른 배치 파일이 call로 불러 쓰는 공통 경로 설정.
rem 엔진 경로가 다르면 환경 변수 KINGDOM_UE_ROOT를 설정해 두면 그 값을 쓴다.

if defined KINGDOM_UE_ROOT (
	set "UE_ROOT=%KINGDOM_UE_ROOT%"
) else (
	set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
)

rem 저장소 루트 = 이 파일이 있는 Tools 폴더의 부모
for %%I in ("%~dp0..") do set "REPO_ROOT=%%~fI"
set "UPROJECT=%REPO_ROOT%\Kingdom.uproject"

if not exist "%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" (
	echo [Kingdom] 엔진을 찾지 못했습니다: %UE_ROOT%
	echo [Kingdom] KINGDOM_UE_ROOT 환경 변수로 엔진 경로를 지정하세요.
	exit /b 1
)
exit /b 0
