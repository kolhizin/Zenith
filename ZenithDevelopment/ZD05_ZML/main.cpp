#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

int main()
{

	std::cout << "Resources freed.\n";
	std::cout << "Closing console in 0.5 second.\n";

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	return 0;
}