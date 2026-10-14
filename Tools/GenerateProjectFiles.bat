@echo off
chcp 65001 >nul
rem VS 솔루션(Kingdom.sln / Kingdom.slnx)을 다시 생성한다.
rem 소스 파일을 추가·삭제·이름 변경한 뒤에 실행하고, VS에서 Reload All.
setlocal
call "%~dp0Environment.bat" || exit /b 1

"%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%UPROJECT%" -game -rocket -progress
set "RESULT=%ERRORLEVEL%"

if "%RESULT%"=="0" (echo [Kingdom] 프로젝트 파일 생성 완료. VS에서 Reload All 하세요.) else (echo [Kingdom] 실패: %RESULT%)
if "%~1"=="" pause
exit /b %RESULT%
