set SDL2DIR=d:\lib\SDL2-2.0.22

rem del /Q /S /F build
rem del /Q /S /F /AHS build
md build
cd build
cmake ..
rem cmake --build . --config Debug

pause
