::::::::::::::::::::::::::::::::::::::::
:: HapRenderer Demo - Hap1 -> DXT1
::::::::::::::::::::::::::::::::::::::::
@echo off
cd "%~dp0"

:: CLSID constants
set CLSID_AsyncReader={E436EBB5-524F-11CE-9F53-0020AF0BA770}
set CLSID_AVISplitter={1B544C20-FD0B-11CE-8C63-00AA0044B51E}
set CLSID_HapDecoder={7A47FC31-8A4D-48B8-8674-F0D2F23BB0BA}
set CLSID_HapRenderer={7DB57FC5-6810-449F-935B-BC9EA3631560}

:: decode sample-1080p30-Hap.avi with HapDecoder, render decoded DXT1 frames with HapRenderer (OpenGL)
bin\dscmd^
 -graph ^
%CLSID_AsyncReader%;src=..\assets\sample-1080p30-Hap.avi,^
%CLSID_AVISplitter%,^
%CLSID_HapDecoder%;file=HapDecoder.ax,^
%CLSID_HapRenderer%;file=bin\HapRenderer.ax^
!0:1,1:2,2:3^
 -noWindow^
 -loop -1^
 -i

echo.
pause
