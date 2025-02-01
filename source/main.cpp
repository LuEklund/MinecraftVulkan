
#include "MvApp.hpp"

//STD
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <crtdbg.h>

#include "MvPerlinNoise.hpp"


int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(824);

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

 	//int* leak = new int[10];

	return EXIT_SUCCESS;
}
