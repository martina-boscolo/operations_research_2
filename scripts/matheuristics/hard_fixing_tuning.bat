@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=1
set SEED_END=5
set PARAM1_START=70
set PARAM1_END=90

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
    for /l %%p in (%PARAM1_START%,10,%PARAM1_END%) do (
        echo Running with seed=%%s, param1=%%p
        ..\..\build\Release\tsp.exe -method HF -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 %%p  -verbose 10 > logs\HF_n%NODES%_seed%%s_param1_%%p.log
        
    )
)

REM Create CSV with headers for each parameter combination
echo 4,> hf_stats.csv
for /l %%p in (%PARAM1_START%,10,%PARAM1_END%) do (
    set "header=!header!,p1=%%p"
)
echo !header! >> hf_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for /l %%p in (%PARAM1_START%,10,%PARAM1_END%) do (
        for /f "tokens=4 delims=;" %%a in ('findstr /C:"$STAT;TWO_OPT" logs\HF_n%NODES%_seed%%s_param1_%%p.log') do (
            set "time=%%a"
            if "!time:~-1!"==";" set "time=!time:~0,-1!"
            set "line=!line!,!time!"
        )
    )
    echo !line! >> hf_stats.csv
)

echo All tasks completed! CSV successfully generated.



