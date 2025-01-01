
#include "MvApp.h"

//STD
#include <iostream>
#include <stdexcept>
#include <cstdlib>


int main()
{
	MvApp App;

	try
	{
		App.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	/*uint32_t ExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);

	std::cout << ExtensionCount << " extensions supported" << std::endl;

	while(!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();
	}*/

	return EXIT_SUCCESS;
}