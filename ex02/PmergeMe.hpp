#ifndef PMERGEME_HPP
# define PMERGEME_HPP

#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <climits>
#include <sys/time.h>

class PmergeMe
{
private:
	// Orthodox Canonical Form - private constructors since this is a utility class
	PmergeMe();
	PmergeMe(const PmergeMe& other);
	PmergeMe& operator=(const PmergeMe& other);
	~PmergeMe();

	// Container-specific Ford-Johnson implementations
	static void fordJohnsonVector(std::vector<int>& a);
	static void fordJohnsonDeque(std::deque<int>& a);

	// Container-specific pairing and sorting helpers
	static void pairAndSortVector(const std::vector<int>& in, std::vector<int>& maxes, std::vector<int>& mins, int& straggler, bool& hasStraggler);
	static void pairAndSortDeque(const std::deque<int>& in, std::deque<int>& maxes, std::deque<int>& mins, int& straggler, bool& hasStraggler);

	// Container-specific binary insertion
	static void binaryInsertVector(std::vector<int>& chain, int value);
	static void binaryInsertDeque(std::deque<int>& chain, int value);

	// Shared helper functions
	static std::vector<size_t> jacobsthalIndices(size_t mCount);
	static bool parsePositiveInt(const std::string& s, int& out);
	static double getTimeDifference(const struct timeval& start, const struct timeval& end);

public:
	// Main public interface
	static bool parseArgs(int argc, char** argv, std::vector<int>& out);
	static void run(const std::vector<int>& input);
};

#endif
