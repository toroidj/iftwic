@echo off
set RETAIL=1
rem *** set value ***
set arcname=iftwic22.zip
set readme=iftwic.txt
set srcname=iftwics.7z

rem *** main ***
md "for View" 2> nul
del /q *.7z 2> NUL
del /q *.zip 2> NUL
del /q *.sp* 2> NUL

del /q *.obj 2> NUL
call m64 -DRELEASE -DFORVIEW
call mvc -DRELEASE -DFORVIEW
for %%i in (*.sp*) do tfilesign sp %%i %%i
for %%i in (*.sp*) do CT %readme% %%i
copy *.sp* "for View"

del /q *.obj 2> NUL
call marm64 -DRELEASE
call m64 -DRELEASE
call mvc -DRELEASE

rem *** Source Archive ***
if %RETAIL%==0 goto :skipsource

for %%i in (*) do CT %readme% %%i
ppb /c %%u/7-zip32.dll,a %srcname% -hide -mx=9 makefile *.vcproj *.sln *.def *.bat *.c *.cpp *.h *.rc *.rh
CT %readme% %srcname%
:skipsource

for %%i in (*.sp*) do tfilesign sp %%i %%i
for %%i in (*.sp*) do CT %readme% %%i

ppb /c %%u/7-ZIP32.DLL,a -tzip -hide -mx=7 %arcname% %readme% *.sp* *.exe %srcname% "for View\*"
tfilesign s %arcname% %arcname%
CT %readme% %arcname%
del /q %srcname% 2> NUL
