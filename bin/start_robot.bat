@echo off

setlocal

set para=x64\Debug

if "%1" NEQ "" set para=%1

set Dir=%~dp0%para%\

@ping -n 1 127.0.0.1>nul
start "sip_robot" /D %Dir% %Dir%sip_robot.exe ..\..\robot.cfg  ..\..\log
 

REM @ping -n 1 127.0.0.1>nul
REM start "cdecoder_server" /D %Dir% %Dir%decoder.exe ..\..\server.cfg  ..\..\log

REM pause
