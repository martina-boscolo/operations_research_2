@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=51
set SEED_END=60

REM Fixed set of parameter values to test
set PARAM_VALUES=10 20 30 40

set NODES=1000
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
    for %%p in (%PARAM_VALUES%) do (
        echo Running with seed=%%s, param1=%%p
        ..\..\build\Release\tsp.exe -method LB -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 %%p -verbose 0 > logs\LB_n%NODES%_seed%%s_param1_%%p.log
    )
)

REM Create CSV with headers for each parameter value
echo 4,k = 10,k = 20,k = 30,k = 40  > lb_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for %%p in (%PARAM_VALUES%) do (
        for /f "tokens=4 delims=;" %%a in ('findstr /C:"$STAT;LB" logs\LB_n%NODES%_seed%%s_param1_%%p.log') do (
            set "time=%%a"
            if "!time:~-1!"==";" set "time=!time:~0,-1!"
            set "line=!line!,!time!"
        )
    )
    echo !line! >> lb_stats.csv
)

echo All tasks completed! CSV successfully generated.



