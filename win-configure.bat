@set SDL2DIR=d:\lib\SDL2-2.0.22

REM rd /S /Q build
REM pause
REM md build

cd build
cmake -L ..
rem cmake --build . --config Debug

pause
