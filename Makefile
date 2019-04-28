all: prog1

prog1: Spheres\main.c
	cl /nologo /W4 Spheres\main.c /link/out:Spheres.exe
	Spheres.exe spheres.ppm