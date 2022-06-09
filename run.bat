@echo off

tcc -run bake.c
call gcc -Ivendor -Isrc -Lvendor src/main.c vendor/glad.c  -o -luser32 -lgdi32 -lopengl32 -o tivi.exe
call tivi.exe
echo "closed with exit code %errorlevel%"
