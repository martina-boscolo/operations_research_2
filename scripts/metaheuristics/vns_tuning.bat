@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=11
set SEED_END=20
set NODES=1000
set TIMELIMIT=60

REM Define specific parameter values (not sequences)
set K=3 5
set R=1 3 5

REM Empty the logs folder if it exists, otherwise create it
if exist logs (
    echo Emptying logs folder...
    del /Q logs\*
) else (
    echo Creating logs folder...
    mkdir logs
)
echo K=%K%
echo R=%R%
REM Generate log files
echo Executing...
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    for %%k in (%K%) do (
        for %%r in (%R%) do (
            echo Running with seed=%%s, param1=%%k, param2=%%r...
            ..\..\build\Release\tsp.exe -method VNS -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 %%k -param2 %%r -verbose 0 > logs\VNS_n%NODES%_seed%%s_k%%k_r%%r.log
        )
    )
)

REM Create CSV header with parameter combinations
echo 6,3-opt x1,3-opt x3,3-opt x5,5-opt x1,5-opt x3,5-opt x5 > vns_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for %%k in (%K%) do (
        for %%r in (%R%) do (
            for /f "tokens=4 delims=;" %%a in ('findstr /C:"$STAT;VNS" logs\VNS_n%NODES%_seed%%s_k%%k_r%%r.log') do (
                set "cost=%%a"
            )
            set "line=!line!,!cost!"
        )
    )
    echo !line! >> vns_stats.csv
)

echo All tasks completed! VNS stats CSV successfully generated.