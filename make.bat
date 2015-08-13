REM change directory to location of batch file
pushd "%~dp0"


REM config for different versions of Visual Studio.
setlocal
set msbuildParameters=/p:Configuration=Release

if "%VisualStudioVersion%" == "11.0" (
    set msbuildParameters=/p:Configuration=Release /toolsversion:12.0 /property:PlatformToolset=v110
)
if "%VisualStudioVersion%" == "12.0" (
    set msbuildParameters=/p:Configuration=Release /toolsversion:12.0 /property:PlatformToolset=v120
)
if "%VisualStudioVersion%" == "14.0" (
    set msbuildParameters=/p:Configuration=Release /toolsversion:14.0 /property:PlatformToolset=v140
)




REM build executable
msbuild windows\HoneyRunner\HoneyRunner.vcxproj %msbuildParameters%



REM copy to correct location
copy windows\HoneyRunner\Release\HoneyRunner.exe .\play_icfp2015.exe

REM be nice and change directory back
popd
