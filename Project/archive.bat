@ECHO OFF
dir *. /b > rarme.txt
for /f %%a in (rarme.txt) do (

REM "Project%1	: resulting archive file"
REM "-m5	: compression level max"
REM "-%%a	: file found in rarme"
REM "-x*	: exclude files"

REM -xipch -x*.sdf -x*.lnk -x*.suo

echo Compressing %%a
"c:\Program Files\WinRAR\WinRar.exe" a Project%1 -m5 -r CrimsonGL.sln %%a/ -xBackups -xData -xDebug -xRelease -xUtilities -xlibs -xipch -xdoc -x%%a/Debug/ -x%%a/Release/
)
del rarme.txt