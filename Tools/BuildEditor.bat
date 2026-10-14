@echo off
rem 더블클릭용 실행기. 실제 내용은 BuildEditor.ps1 (PowerShell)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0BuildEditor.ps1"
set "RESULT=%ERRORLEVEL%"
if "%~1"=="" pause
exit /b %RESULT%
