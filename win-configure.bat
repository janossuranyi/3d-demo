set SDL2DIR=../../lib/SDL2-2.0.22

md build
cd build
cmake -L ..
rem cmake --build . --config Debug

pause
