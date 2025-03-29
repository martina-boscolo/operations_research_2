@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=1
set SEED_END=50
set PARAM_START=0
set PARAM_END=4
set NODES=100
set TIMELIMIT=15

REM Create logs folder if it doesn't exist
if not exist logs mkdir logs

REM Generate log files
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    for /l %%p in (%PARAM_START%,1,%PARAM_END%) do (
        ..\..\tsp.exe -method TS -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param %%p > logs\TS_n%NODES%_seed%%s_param%%p.log
    )
)

REM Create CSV header
echo 5,Param0,Param1,Param2,Param3,Param4 > ts_stats.csv

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



