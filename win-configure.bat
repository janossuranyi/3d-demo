@set SDL2DIR=d:\lib\SDL2-2.0.22

rd /S /Q build
pause

md build
cd build
cmake -L ..
rem cmake --build . --config Debug

pause
