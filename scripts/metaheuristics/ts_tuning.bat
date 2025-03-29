@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=1
set SEED_END=15
set PARAM_START=0
set PARAM_END=4
set NODES=200
set TIMELIMIT=30

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
        ..\..\tsp.exe -method TS -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 %%p -verbose 51 > logs\TS_n%NODES%_seed%%s_param%%p.log
    )
)

REM Create CSV header
echo 5,Fixed min,Fixed max,Random,Linear,Sinusoidal > ts_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for /l %%p in (%PARAM_START%,1,%PARAM_END%) do (
        for /f "tokens=5 delims=;" %%a in (logs\TS_n%NODES%_seed%%s_param%%p.log) do (
            set "cost=%%a"
        )
        set "line=!line!,!cost!"
    )
    echo !line! >> ts_stats.csv
)

echo All tasks completed! CSV successfully generated.



