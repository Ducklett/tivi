@echo off

tcc -run bake.c
call gcc -Ivendor -Isrc -Lvendor src/compilation.c  -o -luser32 -lgdi32 -lopengl32 -o tivi.exe
call tivi.exe
echo "closed with exit code %errorlevel%"
