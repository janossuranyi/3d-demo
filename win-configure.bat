set SDL2DIR=../../lib/SDL2-2.0.22

md ..\build-3d-demo
cd ..\build-3d-demo
cmake -L ..\3d-demo-git
rem cmake --build . --config Debug

pause
