/*
	cl /nologo /W4 main.c /link/out:Spheres.exe
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#undef min
#undef max

#define WIDTH 2000
#define HEIGHT 1500

#define PI 3.141592653589793

struct sphere {
	double r;
	double pos[3];
	double color[3];
};

double min(double a, double b)
{
	if (a < b)
		return a;
	else
		return b;
}

double max(double a, double b)
{
	if (a >= b)
		return a;
	else
		return b;
}

double dot(double a[3], double b[3])
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void normalize(double n[3], double v[3])
{
	double r = sqrt(dot(v, v));
	n[0] = v[0] / r;
	n[1] = v[1] / r;
	n[2] = v[2] / r;
}

double calcDiscriminant(double origin[3], double sphere[3], double dir[3], double r)
{
	double u[3];
	u[0] = origin[0] - sphere[0];
	u[1] = origin[1] - sphere[1];
	u[2] = origin[2] - sphere[2];
	return 4.0*dot(u, dir)* dot(u, dir) - 4.0*(dot(u, u) - r*r);
}

double calcSpereIntersection(double origin[3], double sphere[3], double dir[3], double discriminant)
{
	double u[3];
	u[0] = origin[0] - sphere[0];
	u[1] = origin[1] - sphere[1];
	u[2] = origin[2] - sphere[2];
	
	double t = (-2.0*dot(u, dir) - sqrt(discriminant))/2.0;
	return max(t, 0);
}


int render(char* pixels, unsigned int numSpheres, struct sphere* s)
{
	double r=0.0, g=0.0, b=0.0;
	double origin[] = { 0.0, 0.0, 0.0 };
	double lightSource[] = { -50.0, 0.0, -100.0 };
	double tmp[3];
	double dir[3];
	double t = 12000.0;
	double P_hit[3];
	double normalSphere[3];
	double lightDir[3];
	double diffuseIntensity;
	for (unsigned int i = 0; i < HEIGHT; i++)
	{
		for (unsigned int j = 0; j < WIDTH; j++)
		{
			double x = (2.0 * (double)j / (double)WIDTH - 1.0) * 0.2 * WIDTH/HEIGHT;
			double y = -(2.0 * (double)i / (double)HEIGHT - 1.0) * 0.2;
			double z = 0.5;
			double t0 = 10000;
			tmp[0] = x;
			tmp[1] = y;
			tmp[2] = z;
			normalize(dir, tmp);
			t = 12000;
			for (unsigned int k = 0; k < numSpheres; k++)
			{
				double discr = calcDiscriminant(origin, s[k].pos, dir, s[k].r);
				if (discr >= 0.0)
				{
					t0 = calcSpereIntersection(origin, s[k].pos, dir, discr);
					if (t < t0) continue;
					t = t0;
					//printf("t0 = %f\n", t0);
					P_hit[0] = t0 * dir[0];
					P_hit[1] = t0 * dir[1];
					P_hit[2] = t0 * dir[2];
					tmp[0] = P_hit[0] - s[k].pos[0];
					tmp[1] = P_hit[1] - s[k].pos[1];
					tmp[2] = P_hit[2] - s[k].pos[2];
					normalize(normalSphere, tmp);
					tmp[0] = lightSource[0] - P_hit[0];
					tmp[1] = lightSource[1] - P_hit[1];
					tmp[2] = lightSource[2] - P_hit[2];
					normalize(lightDir, tmp);					
					diffuseIntensity = max(dot(lightDir, normalSphere), 0.2);
					for (unsigned int l = 0; l < numSpheres; l++)
					{
						discr = calcDiscriminant(P_hit, s[l].pos, lightDir, s[l].r);
						if (discr >= 0 && k != l && calcSpereIntersection(P_hit, s[l].pos, lightDir, discr) >0 )
						{
							r = s[k].color[0] * 0.2;
							g = s[k].color[1] * 0.2;
							b = s[k].color[2] * 0.2;
							break;
						}
						else
						{
							r = s[k].color[0] * diffuseIntensity;
							g = s[k].color[1] * diffuseIntensity;
							b = s[k].color[2] * diffuseIntensity;
						}
					}
				}
			}
			if (t >= 12000)
			{
				r = 0.0;
				g = 0.0;
				b = 0.0;
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
	const char* width = "2000\n";
	const char* height = "1500\n";
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
	printf("*** RayTracing Spheres ***\n\n");
	if (argc == 1)
	{
		printf("Filename fehlt!: ");
		printf("%s filename\n", argv[0]);
		return 1;
	}
	size_t pixelSize = 3 * WIDTH * HEIGHT;
	char* pixels = calloc(1, pixelSize);
	memset(pixels, 255, pixelSize);

	struct sphere s1 = { 0.5, { -1.0, 0.0, 4.0 }, { 0.0, 1.0, 0.0} };
	struct sphere s2 = { 1.0, { -0.5, -0.5, 6.0 }, { 1.0, 0.0, 0.0} };
	struct sphere s3 = { 4.0, { 1.0,  0.0, 12.0 }, { 1.0, 1.0, 0.0} };

	struct sphere s[3];
	s[0] = s1;
	s[1] = s2;
	s[2] = s3;

	render(pixels, 3, s);
	writeFile(argv[1], pixels, pixelSize);

	return 0;
}