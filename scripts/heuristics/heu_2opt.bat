@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=1
set SEED_END=10
set NODES=1000
set TIMELIMIT=60

REM Define specific parameter values
set METHOD=NN EM

REM Empty the logs folder if it exists, otherwise create it
if exist logs (
    echo Emptying logs folder...
    del /Q logs\*
) else (
    echo Creating logs folder...
    mkdir logs
)
echo METHOD=%METHOD%

REM Generate log files
echo Executing...
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    for %%m in (%METHOD%) do (
        echo Running with method=%%m, seed=%%s...
        ..\..\build\Release\tsp.exe -method %%m -n %NODES% -seed %%s -timelimit %TIMELIMIT% -verbose 0 -param1 1 > logs\%%m_n%NODES%_seed%%s.log
    )
)

REM Create CSV header with parameter combinations
echo 2,NN,EM > heu_2opt.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for %%m in (%METHOD%) do (
        for /f "tokens=4 delims=;" %%a in ('findstr /C:"$STAT;" logs\%%m_n%NODES%_seed%%s.log') do (
            set "cost=%%a"
        )
        set "line=!line!,!cost!"
    )
    echo !line! >> heu_2opt.csv
)

echo All tasks completed! Refined heuristic comparison CSV successfully generated.