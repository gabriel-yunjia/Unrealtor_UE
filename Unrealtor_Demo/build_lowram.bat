@echo off
setlocal enabledelayedexpansion

set PROJECT_DIR=%~dp0
set PROJECT_FILE=%PROJECT_DIR%Unrealtor_Demo.uproject

if not exist "%PROJECT_FILE%" (
    echo Build_Project_LowRAM: Unrealtor_Demo.uproject not found in %PROJECT_DIR%
    pause
    exit /b 1
)

set UE_ROOT=

for /f "tokens=2,*" %%a in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.6" /v "InstalledDirectory" 2^>nul') do (
    if "%%a"=="REG_SZ" set "UE_ROOT=%%b"
)

if not defined UE_ROOT (
    for /f "tokens=2,*" %%a in ('reg query "HKCU\SOFTWARE\EpicGames\Unreal Engine\5.6" /v "InstalledDirectory" 2^>nul') do (
        if "%%a"=="REG_SZ" set "UE_ROOT=%%b"
    )
)

if not defined UE_ROOT (
    for %%p in (
        "C:\Program Files\Epic Games\UE_5.6"
        "D:\Program Files\Epic Games\UE_5.6"
        "C:\Epic Games\UE_5.6"
        "D:\Epic Games\UE_5.6"
        "C:\UE5\UE_5.6"
        "D:\UE5\UE_5.6"
    ) do (
        if not defined UE_ROOT (
            if exist "%%~p\Engine\Build\BatchFiles\Build.bat" set "UE_ROOT=%%~p"
        )
    )
)

if not defined UE_ROOT (
    echo Build_Project_LowRAM: Could not locate Unreal Engine 5.6.
    echo.
    echo Checked registry keys under SOFTWARE\EpicGames\Unreal Engine\5.6
    echo Checked common install paths on C: and D: drives.
    echo.
    echo If your engine is installed elsewhere, set UE_ROOT in this script.
    pause
    exit /b 1
)

if "%UE_ROOT:~-1%"=="\" set "UE_ROOT=%UE_ROOT:~0,-1%"

set BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat
set EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe

if not exist "%BUILD_BAT%" (
    echo Build_Project_LowRAM: Build.bat not found at expected path.
    echo   %BUILD_BAT%
    echo Engine root resolved to: %UE_ROOT%
    pause
    exit /b 1
)

echo Build_Project_LowRAM: Engine root: %UE_ROOT%
echo.
echo Build_Project_LowRAM: Low-memory mode enabled.
echo Build_Project_LowRAM: Limiting compile parallelism with -MaxParallelActions=2
echo.

echo Build_Project_LowRAM: Compiling Unrealtor_DemoEditor [Win64 ^| Development]...
echo.
call "%BUILD_BAT%" Unrealtor_DemoEditor Win64 Development -Project="%PROJECT_FILE%" -WaitMutex -MaxParallelActions=2
set BUILD_EXIT=!ERRORLEVEL!

if !BUILD_EXIT! NEQ 0 (
    echo.
    echo Build_Project_LowRAM: Compilation failed [exit code !BUILD_EXIT!].
    echo Review the output above for errors.
    pause
    exit /b !BUILD_EXIT!
)

echo.
echo Build_Project_LowRAM: Build succeeded.

set /p LAUNCH="Launch editor? [Y/N] "
if /i "%LAUNCH%"=="Y" (
    start "" "%EDITOR%" "%PROJECT_FILE%"
)
