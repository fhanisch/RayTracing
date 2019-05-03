/*
	cl /nologo /W4 main.c /link/out:ppmImage.exe
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 2160
#define HEIGHT 2160

#define PI 3.141592653589793

int createImage(char* pixels)
{
	double r, g, b;
	for (unsigned int i = 0; i < HEIGHT; i++)
	{
		for (unsigned int j = 0; j < WIDTH; j++)
		{
			double x = 2.0 * (double)j / (double)WIDTH - 1.0;
			double y = -(2.0 * (double)i / (double)HEIGHT - 1.0);
			double R = sqrt(pow(x, 2.0) + pow(y, 2.0));
			double phi = asin(y / R);

			if (R <= 0.75 && R >= 0.2)
			{
				r = 0.0;
				g = 0.0;
				b = 1.0 - R;
			}
			else if (R > 0.75 && R <= 1.0 && phi >= -PI / 4.0 && phi <= PI / 2.0 && x <= 0.0)
			{
				r = (phi + PI / 2.0) / PI;
				g = 1.0;
				b = 0.0;
			}
			else if (R > 0.75 && R <= 1.0 && phi >= -PI / 4.0 && phi <= PI / 2.0 && x >= 0.0)
			{
				r = 1.0;
				g = (phi + PI / 2.0) / PI;
				b = 0.0;
			}
			else
			{
				r = 0.2;
				g = 0.2;
				b = 0.2;
			}


			pixels[0] = (char)(r * 255.0);
			pixels[1] = (char)(g * 255.0);
			pixels[2] = (char)(b * 255.0);
			pixels += 3;
		}
	}

	return 0;
}

int writeFile(const char* filename, char* pixels, size_t pixelSize)
{
	FILE* file;
	const char* magic = "P6\n";
	const char* width = "2160\n";
	const char* height = "2160\n";
	const char* hellikeit = "255\n";

	file = fopen(filename, "wb");
	fwrite(magic, strlen(magic), 1, file);
	fwrite(width, strlen(width), 1, file);
	fwrite(height, strlen(height), 1, file);
	fwrite(hellikeit, strlen(hellikeit), 1, file);
	fwrite(pixels, pixelSize, 1, file);
	fclose(file);

	return 0;
}

int main(int argc, char* argv[])
{
	printf("*** PPM - Image ***\n\n");
	if (argc == 1)
	{
		printf("Filename fehlt!: ");
		printf("%s filename\n", argv[0]);
		return 1;
	}	
	size_t pixelSize = 3 * WIDTH * HEIGHT;
	char* pixels = calloc(1, pixelSize);
	memset(pixels, 255, pixelSize);
	createImage(pixels);
	writeFile(argv[1], pixels, pixelSize);

	return 0;
}