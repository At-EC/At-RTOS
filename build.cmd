@echo off

powershell -ExecutionPolicy Bypass -file "%~dp0Build.ps1 %*"

set PATH=%~dp0.tools;%PATH%
