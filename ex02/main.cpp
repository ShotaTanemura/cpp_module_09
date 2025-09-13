#include "PmergeMe.hpp"

int main(int argc, char** argv)
{
	std::vector<int> input;

	// Parse and validate arguments
	if (!PmergeMe::parseArgs(argc, argv, input))
	{
		std::cerr << "Error" << std::endl;
		return 1;
	}

	// Run the sorting algorithm with both containers
	PmergeMe::run(input);
	
	return 0;
}
