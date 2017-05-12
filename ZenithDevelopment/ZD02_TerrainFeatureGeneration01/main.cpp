#include <iostream>

#include <random>

int main()
{
	std::mt19937 gen;
	gen.seed(3);
	std::poisson_distribution<> pd(3);

	for(uint16_t i = 0; i < 20; i++)
		std::cout << pd(gen) << "\n";

	std::cout << "\nTest 2";
	return 0;
}