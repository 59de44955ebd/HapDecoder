::::::::::::::::::::::::::::::::::::::::
:: HapRenderer Demo - Hap Q
::::::::::::::::::::::::::::::::::::::::
@echo off
cd "%~dp0"

:: CLSID constants
set CLSID_AsyncReader={E436EBB5-524F-11CE-9F53-0020AF0BA770}
set CLSID_AVISplitter={1B544C20-FD0B-11CE-8C63-00AA0044B51E}
set CLSID_HapDecoder={7A47FC31-8A4D-48B8-8674-F0D2F23BB0BA}
set CLSID_VideoRendererDefault={6BC1CFFA-8FC1-4261-AC22-CFB4CC38DB50}

:: decode sample-1080p30-HapQ.avi with HapDecoder (via CPU), render with VMR-7
bin\dscmd^
 -graph ^
%CLSID_AsyncReader%;src=..\assets\sample-1080p30-HapQ.avi,^
%CLSID_AVISplitter%,^
%CLSID_HapDecoder%;file=HapDecoder.ax,^
%CLSID_VideoRendererDefault%^
!0:1,1:2,2:3^
 -winCaption "HapRenderer Hap Q Demo"^
 -loop -1^
 -i

echo.
pause
