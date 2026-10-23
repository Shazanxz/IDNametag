@echo off
setlocal
rem Compila IDNameTag.asi (x86 / 32-bit, obrigatorio para o GTA SA).
rem Requer o Visual Studio com a carga "Desenvolvimento para desktop com C++".

set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq delims=" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VSDIR=%%i
if not defined VSDIR (
    echo Ferramentas C++ do Visual Studio nao encontradas.
    echo Abra o Visual Studio Installer e instale "Desenvolvimento para desktop com C++".
    exit /b 1
)

call "%VSDIR%\VC\Auxiliary\Build\vcvars32.bat" >nul || exit /b 1

if not exist out mkdir out
cl /nologo /O2 /MT /LD /EHsc /W3 /DNDEBUG src\main.cpp /Fo"out\\" /Fe"out\IDNameTag.asi" /link user32.lib kernel32.lib || exit /b 1
del /q out\*.obj out\*.exp out\*.lib 2>nul

echo.
echo Pronto: out\IDNameTag.asi
