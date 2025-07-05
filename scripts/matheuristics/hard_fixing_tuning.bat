@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=51
set SEED_END=60

REM Fixed set of parameter values to test
set PARAM_VALUES=1 40 50 60 80 

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
        ..\..\build\Release\tsp.exe -method HF -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 %%p -verbose 0 > logs\HF_n%NODES%_seed%%s_param1_%%p.log
    )
)

REM Create CSV with headers for each parameter value
echo 5,Random,40%%,50%%,60%%,80%%  > hf_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for %%p in (%PARAM_VALUES%) do (
        for /f "tokens=4 delims=;" %%a in ('findstr /C:"$STAT;HF" logs\HF_n%NODES%_seed%%s_param1_%%p.log') do (
            set "time=%%a"
            if "!time:~-1!"==";" set "time=!time:~0,-1!"
            set "line=!line!,!time!"
        )
    )
    echo !line! >> hf_stats.csv
)

echo All tasks completed! CSV successfully generated.



