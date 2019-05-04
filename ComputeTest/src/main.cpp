#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "Vulkan.h"

#define APP_NAME "Compute Test"
#define ENGINE_NAME "MyVulkanEngine"
#define IMG_WIDTH 3840
#define IMG_HEIGHT 2160

#define STRINGIFY(str) #str
#define TO_STRING(str) STRINGIFY(str\n)

class App
{
	Vulkan* vulkan;
public:
	App(int argc, char* argv[])
	{
		vulkan = new Vulkan(APP_NAME, ENGINE_NAME, IMG_WIDTH, IMG_HEIGHT);
	}
	~App()
	{
		vulkan->~Vulkan();
		std::cout << "Auf Wiedersehen !!!" << std::endl;
	}
	void run()
	{
		vulkan->submitCommandBuffer();
	}
	int saveImage(const char* filename)
	{
		FILE* file;
		const char* magic = "P6\n";
		const char* width = TO_STRING(IMG_WIDTH);
		const char* height = TO_STRING(IMG_HEIGHT);
		const char* hellikeit = "255\n";
		size_t pixelSize = 3 * IMG_WIDTH * IMG_HEIGHT;
		char* pixels = (char*)calloc(1, pixelSize);
		
		std::cout << "Save Image." << std::endl;
		memset(pixels, 255, pixelSize);
		vulkan->getPixelBuffer(pixels);

		file = fopen(filename, "wb");
		fwrite(magic, strlen(magic), 1, file);
		fwrite(width, strlen(width), 1, file);
		fwrite(height, strlen(height), 1, file);
		fwrite(hellikeit, strlen(hellikeit), 1, file);
		fwrite(pixels, pixelSize, 1, file);
		fclose(file);

		return 0;
	}
};

int main(int argc, char* argv[])
{
	std::cout << "*** " << APP_NAME << " ***" << std::endl;
	App app(argc, argv);
	app.run();
	app.saveImage("Kugeln.ppm");

	return 0;
}