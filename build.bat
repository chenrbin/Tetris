cmake --build build --config release
del Tetris.exe
echo %cd%
copy ".\build\bin\Release\Tetris.exe" "Tetris.exe"