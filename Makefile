all: prog1

prog1:
	cl /nologo /W4 Spheres\main.c /link/out:Spheres.exe
	Spheres.exe spheres.ppm

shader:
	glslangValidator -V ComputeTest\src\mandelbrot.comp -o x64\Debug\mandelbrot.spv
	glslangValidator -V ComputeTest\src\powermeter.comp -o x64\Debug\powermeter.spv
	glslangValidator -V ComputeTest\src\spheres.comp -o x64\Debug\spheres.spv