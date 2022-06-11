@echo off

tcc -run bake.c
call gcc -O3 -s -mwindows -Ivendor -Isrc -Lvendor src/compilation.c icon.res -o -luser32 -lgdi32 -lopengl32 -o tivi.exe
