#include <iostream>
#include "Vulkan.h"

#define APP_NAME "Compute Test"
#define ENGINE_NAME "MyVulkanEngine"
#define IMG_WIDTH 3840
#define IMG_HEIGHT 2160

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
};

int main(int argc, char* argv[])
{
	std::cout << "*** " << APP_NAME << " ***" << std::endl;
	App app(argc, argv);

	return 0;
}