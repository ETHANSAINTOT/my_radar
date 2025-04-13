@echo off
REM === Compilation sous Windows avec MinGW + CSFML ===

REM Répertoires d'includes et de lib (à adapter en fonction de l'installation CSFML)
set CSFML_INCLUDE=C:\CSFML\include
set CSFML_LIB=C:\CSFML\lib

REM Compilation
gcc src\main.c src\parser.c src\simulation.c src\update_render.c -Iinclude -I%CSFML_INCLUDE% -L%CSFML_LIB% -lcsfml-graphics -lcsfml-window -lcsfml-system -o my_radar.exe

IF %ERRORLEVEL% NEQ 0 (
    echo ❌ Compilation échouée.
) ELSE (
    echo ✅ Compilation réussie. Exécutable : my_radar.exe
)

pause
