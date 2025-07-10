@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=61
set SEED_END=70

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

    echo Running HF with seed=%%s, param1=40
    ..\..\build\Release\tsp.exe -method HF -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 40 -verbose 0 > logs\HF_n%NODES%_seed%%s.log
    
    echo Running LB with seed=%%s, param1=20
    ..\..\build\Release\tsp.exe -method LB -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 20 -verbose 0 > logs\LB_n%NODES%_seed%%s.log

)

set METHOD=HF LB

REM Create CSV with headers for each parameter value
echo 2,HF,LB > matheu_comp.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for %%m in (%METHOD%) do (
        for /f "tokens=4 delims=;" %%a in ('findstr /C:"$STAT;" logs\%%m_n%NODES%_seed%%s.log') do (
            set "time=%%a"
            if "!time:~-1!"==";" set "time=!time:~0,-1!"
            set "line=!line!,!time!"
        )
    )
    echo !line! >> matheu_comp.csv
)

echo All tasks completed! CSV successfully generated.



