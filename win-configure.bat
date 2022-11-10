set SDL2DIR=d:\lib\SDL2-2.0.22

# del /Q /S /F build
# del /Q /S /F /AHS build
md build
cd build
cmake -L ..
rem cmake --build . --config Debug

pause
