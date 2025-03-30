@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=1
set SEED_END=3
set NODES=50
set TIMELIMIT=5

REM Define specific parameter values (not sequences)
set K=3 5
set R=1 3 5 10

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
            ..\..\tsp.exe -method VNS -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 %%k -param2 %%r -verbose 51 > logs\VNS_n%NODES%_seed%%s_param1_%%k_param2_%%r.log
        )
    )
)

REM Create CSV header with parameter combinations
echo 8,k=3_r=1,k=3_r=3,k=3_r=5,k=3_r=10,k=5_r=1,k=5_r=3,k=5_r=5,k=5_r=10 > vns_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for %%k in (%K%) do (
        for %%r in (%R%) do (
            for /f "tokens=6 delims=;" %%a in ('findstr /C:"$STAT;VNS" logs\VNS_n%NODES%_seed%%s_param1_%%k_param2_%%r.log') do (
                set "cost=%%a"
            )
            set "line=!line!,!cost!"
        )
    )
    echo !line! >> vns_stats.csv
)

echo All tasks completed! VNS stats CSV successfully generated.