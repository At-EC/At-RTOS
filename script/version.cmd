@ECHO OFF

pushd "%~dp0"
powershell -ExecutionPolicy bypass -file "%~dpn0.ps1" %*

popd