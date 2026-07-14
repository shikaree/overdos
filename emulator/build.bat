@echo off
rem ---------------------------------------------------------------------------
rem  Build the OverDOS emulator (overdos.exe) on Windows with MinGW-w64 gcc.
rem  A convenience wrapper around the same compile the Makefile runs; if you
rem  have `make`, `mingw32-make` works too.
rem ---------------------------------------------------------------------------
setlocal enabledelayedexpansion

rem Use gcc from PATH if present, otherwise fall back to the winget install.
where gcc >nul 2>nul
if errorlevel 1 (
  set "GCCBIN=%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
  if exist "!GCCBIN!\gcc.exe" set "PATH=!GCCBIN!;%PATH%"
)

gcc -O2 -Wall ^
    src\callback.c src\console.c src\elf.c src\exit.c src\init.c ^
    src\isa_dec.c src\isa_exec.c src\main.c src\mmu.c ^
    -o overdos.exe
if errorlevel 1 (
    echo BUILD FAILED
    exit /b 1
)
echo Built overdos.exe  -  run it with:  overdos.exe
endlocal
