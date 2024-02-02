@echo off

setlocal

set para=x64\Release

if "%1" NEQ "" set para=%1

set Dir=%~dp0%para%\

REM @ping -n 1 127.0.0.1>nul
REM start "video_split" /D %Dir% %Dir%cvideo_split.exe ..\..\server.cfg  ..\..\log
 

@ping -n 1 127.0.0.1>nul
start "cdecoder_server" /D %Dir% %Dir%decoder.exe ..\..\server.cfg  ..\..\log

REM pause
