@echo off
setlocal enabledelayedexpansion

REM Define constants
set SEED_START=1
set SEED_END=5
set PARAM1_START=0
set PARAM1_END=2
set PARAM2_START=0
set PARAM2_END=1
set NODES=200
set TIMELIMIT=60

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
    for /l %%p in (%PARAM1_START%,1,%PARAM1_END%) do (
        for /l %%q in (%PARAM2_START%,1,%PARAM2_END%) do (
            echo Running with seed=%%s, param1=%%p, param2=%%q...
            ..\..\build\Release\tsp.exe -method BC -n %NODES% -seed %%s -timelimit %TIMELIMIT% -param1 %%p -param2 %%q -verbose 10 > logs\BC_n%NODES%_seed%%s_param1_%%p_param2_%%q.log
        )
    )
)

REM Create CSV with headers for each parameter combination
echo seed,> bc_stats.csv
set "header=seed"
for /l %%p in (%PARAM1_START%,1,%PARAM1_END%) do (
    for /l %%q in (%PARAM2_START%,1,%PARAM2_END%) do (
        set "header=!header!,p1=%%p_p2=%%q"
    )
)
echo !header! >> bc_stats.csv

REM Extract data and populate CSV
for /l %%s in (%SEED_START%,1,%SEED_END%) do (
    set "line=%%s"
    for /l %%p in (%PARAM1_START%,1,%PARAM1_END%) do (
        for /l %%q in (%PARAM2_START%,1,%PARAM2_END%) do (
            for /f "tokens=5 delims=;" %%a in ('findstr /C:"$STAT;BranchAndCut" logs\BC_n%NODES%_seed%%s_param1_%%p_param2_%%q.log') do (
                set "time=%%a"
                if "!time:~-1!"==";" set "time=!time:~0,-1!"
                set "line=!line!,!time!"
            )
        )
    )
    echo !line! >> bc_stats.csv
)

echo All tasks completed! CSV successfully generated.



