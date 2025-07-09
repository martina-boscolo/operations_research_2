@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=31
set SEED_END=40
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

    echo Running with seed=%%s, param1=0, param2=0, param3=0...
    ..\..\build\Release\tsp.exe -method BC -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 0 -param2 0 -param3 0 -verbose 0 > logs\BC_n%NODES%_seed%%s_p1_0_p2_0_p3_0.log
    
    echo Running with seed=%%s, param1=1, param2=0, param3=0...
    ..\..\build\Release\tsp.exe -method BC -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 1 -param2 0 -param3 0 -verbose 0 > logs\BC_n%NODES%_seed%%s_p1_1_p2_0_p3_0.log
    
    echo Running with seed=%%s, param1=0, param2=1, param3=0...
    ..\..\build\Release\tsp.exe -method BC -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 0 -param2 1 -param3 0 -verbose 0 > logs\BC_n%NODES%_seed%%s_p1_0_p2_1_p3_0.log
    
    echo Running with seed=%%s, param1=0, param2=0, param3=1...
    ..\..\build\Release\tsp.exe -method BC -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 0 -param2 0 -param3 1 -verbose 0 > logs\BC_n%NODES%_seed%%s_p1_0_p2_0_p3_1.log

    echo Running with seed=%%s, param1=1, param2=1, param3=1...
    ..\..\build\Release\tsp.exe -method BC -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 1 -param2 1 -param3 1 -verbose 0 > logs\BC_n%NODES%_seed%%s_p1_1_p2_1_p3_1.log
)

REM Create CSV with headers for each parameter combination
echo 5,Base,Only Warm-up,Only Frac-SECs,Only Post-heu,All > bc_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"

    for /f "tokens=5 delims=;" %%a in ('findstr /C:"$STAT;BC" logs\BC_n%NODES%_seed%%s_p1_0_p2_0_p3_0.log') do (
        set "time=%%a"
        if "!time:~-1!"==";" set "time=!time:~0,-1!"
        set "line=!line!,!time!"
    )

    for /f "tokens=5 delims=;" %%a in ('findstr /C:"$STAT;BC" logs\BC_n%NODES%_seed%%s_p1_1_p2_0_p3_0.log') do (
        set "time=%%a"
        if "!time:~-1!"==";" set "time=!time:~0,-1!"
        set "line=!line!,!time!"
    )

    for /f "tokens=5 delims=;" %%a in ('findstr /C:"$STAT;BC" logs\BC_n%NODES%_seed%%s_p1_0_p2_1_p3_0.log') do (
        set "time=%%a"
        if "!time:~-1!"==";" set "time=!time:~0,-1!"
        set "line=!line!,!time!"
    )

    for /f "tokens=5 delims=;" %%a in ('findstr /C:"$STAT;BC" logs\BC_n%NODES%_seed%%s_p1_0_p2_0_p3_1.log') do (
        set "time=%%a"
        if "!time:~-1!"==";" set "time=!time:~0,-1!"
        set "line=!line!,!time!"
    )

    for /f "tokens=5 delims=;" %%a in ('findstr /C:"$STAT;BC" logs\BC_n%NODES%_seed%%s_p1_1_p2_1_p3_1.log') do (
        set "time=%%a"
        if "!time:~-1!"==";" set "time=!time:~0,-1!"
        set "line=!line!,!time!"
    )

    echo !line! >> bc_stats.csv
)

echo All tasks completed! CSV successfully generated.



