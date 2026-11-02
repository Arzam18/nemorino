echo off
if "%PATH%"=="%PATH:Visual=%" (
call "D:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
echo on
set PPD="USE_AVX2"
if "%1%"=="AVX2" set PPD="USE_AVX2"
if "%1%"=="AVX2_PEXT" set PPD="PEXT;USE_AVX2"
if "%1%"=="SSE2" set PPD="USE_SSE2" 
if "%1%"=="SSE41" set PPD="USE_SSE2;USE_SSE41"
if "%1%"=="OLD_CPU" set PPD=""
echo %PPD%
msbuild Nelson\Nelson.vcxproj -target:Clean -property:Configuration=PGO
set /p Build=<buildnumber.txt
set /a Build=%Build%+1
echo %Build%>buildnumber.txt
echo #pragma once>Nelson\version.h
echo #define BUILD_NUMBER %Build%>>Nelson\version.h
MSBuild.exe Nelson\Nelson.vcxproj -property:Configuration=PGO;WholeProgramOptimization=PGInstrument;LinkTimeCodeGeneration=PGInstrument;PostBuildEventUseInBuild=false;DefineConstants=%PPD%
.\Nelson\x64\PGO\nemorino bench 10
.\Nelson\x64\PGO\nemorino bench4 10
MSBuild.exe Nelson\Nelson.vcxproj -property:Configuration=PGO;WholeProgramOptimization=PGOptimize;LinkTimeCodeGeneration=PGOptimization;PostBuildEventUseInBuild=false;DefineConstants=%PPD%
move /Y Nelson\x64\PGO\nemorino.exe Nelson\x64\PGO\nemorino_%1%.exe
move /Y Nelson\x64\PGO\nemorino.pdb Nelson\x64\PGO\nemorino_%1%.pdb
copy /Y Nelson\x64\PGO\nemorino_%1%.exe \\BACH\Documents\EngineDev\Builds\
copy /Y Nelson\x64\PGO\nemorino_%1%.pdb \\BACH\Documents\EngineDev\Builds\
cd Nelson
"D:\Program Files\7-Zip\7z.exe" a src.zip *.h *.cpp
cd ..
copy /Y Nelson\src.zip \\BACH\Documents\EngineDev\Builds\
if "%1%"=="AVX2" copy /Y Nelson\x64\PGO\nemorino_%1%.exe \\BACH\Documents\EngineDev\Builds\nemorino.exe