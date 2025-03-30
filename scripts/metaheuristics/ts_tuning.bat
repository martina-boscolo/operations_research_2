@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=1
set SEED_END=3
set PARAM_START=0
set PARAM_END=4
set NODES=200
set TIMELIMIT=10

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
    for /l %%p in (%PARAM_START%,1,%PARAM_END%) do (
        echo Running with seed=%%s, param=%%p...
        ..\..\tsp.exe -method TS -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 %%p -verbose 41 > logs\TS_n%NODES%_seed%%s_param%%p.log
    )
)

REM Create CSV header
echo 5,Fixed min,Fixed max,Random,Linear,Sinusoidal > ts_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for /l %%p in (%PARAM_START%,1,%PARAM_END%) do (
        for /f "tokens=4 delims=;" %%a in ('findstr /C:"$STAT;TS" logs\TS_n%NODES%_seed%%s_param%%p.log') do (
            set "cost=%%a"
        )
        set "line=!line!,!cost!"
    )
    echo !line! >> ts_stats.csv
)

echo All tasks completed! CSV successfully generated.



