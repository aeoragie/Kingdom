@echo off
chcp 65001 >nul
rem 에디터 타깃(KingdomEditor, Win64, Development)을 빌드한다.
rem 에디터가 켜져 있으면 Live Coding과 충돌할 수 있으니 에디터를 닫고 실행한다.
setlocal
call "%~dp0Environment.bat" || exit /b 1

call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" KingdomEditor Win64 Development -project="%UPROJECT%" -waitmutex
set "RESULT=%ERRORLEVEL%"

if "%RESULT%"=="0" (echo [Kingdom] 에디터 빌드 완료.) else (echo [Kingdom] 실패: %RESULT%)
if "%~1"=="" pause
exit /b %RESULT%
