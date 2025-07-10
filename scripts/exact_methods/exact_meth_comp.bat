@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=41
set SEED_END=50
set NODES=300
set TIMELIMIT=180

REM Empty the logs folder if it exists, otherwise create it
if exist logs (
    echo Emptying logs folder...
    del /Q logs\*
) else (
    echo Creating logs folder...
    mkdir logs
)
REM Generate log files
echo Executing...
for /l %%s in (%SEED_START%,1,%SEED_END%) do (

    echo Running B with seed=%%s...
    ..\..\build\Release\tsp.exe -method BL -n %NODES% -seed %%s -timelimit %TIMELIMIT% -verbose 0 > logs\B_n%NODES%_seed%%s.log
    
    echo Running BC with seed=%%s, param1=0, param2=1, param3=1...
    ..\..\build\Release\tsp.exe -method BC -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 0 -param2 1 -param3 1 -verbose 0 > logs\BC_n%NODES%_seed%%s.log
)

REM Create CSV with headers for each parameter combination
echo 2,BL,BC > exm_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"

    for /f "tokens=5 delims=;" %%a in ('findstr /C:"$STAT;BL" logs\B_n%NODES%_seed%%s.log') do (
        set "time=%%a"
        if "!time:~-1!"==";" set "time=!time:~0,-1!"
        set "line=!line!,!time!"
    )

    for /f "tokens=5 delims=;" %%a in ('findstr /C:"$STAT;BC" logs\BC_n%NODES%_seed%%s.log') do (
        set "time=%%a"
        if "!time:~-1!"==";" set "time=!time:~0,-1!"
        set "line=!line!,!time!"
    )

    echo !line! >> exm_stats.csv
)

echo All tasks completed! CSV successfully generated.