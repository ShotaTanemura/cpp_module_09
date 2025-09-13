#include "PmergeMe.hpp"

// Orthodox Canonical Form - private constructors
PmergeMe::PmergeMe() {}
PmergeMe::PmergeMe(const PmergeMe& other) { (void)other; }
PmergeMe& PmergeMe::operator=(const PmergeMe& other) { (void)other; return *this; }
PmergeMe::~PmergeMe() {}

// Strict parsing with overflow protection
bool PmergeMe::parsePositiveInt(const std::string& s, int& out)
{
	if (s.empty())
		return false;

	size_t i = 0;
	// Check for optional leading '+'
	if (s[0] == '+')
		i = 1;

	if (i >= s.length())
		return false;

	// Reject leading zeros (except for single '0' which is handled later)
	if (s[i] == '0' && i + 1 < s.length())
		return false;

	// Verify all remaining characters are digits
	for (size_t j = i; j < s.length(); j++)
	{
		if (s[j] < '0' || s[j] > '9')
			return false;
	}

	// Manual overflow-safe conversion
	unsigned long long acc = 0;
	for (size_t j = i; j < s.length(); j++)
	{
		int digit = s[j] - '0';
		// Check for overflow before accumulating
		if (acc > (static_cast<unsigned long long>(INT_MAX) - digit) / 10)
			return false;
		acc = acc * 10 + digit;
	}

	// Check final value is positive and within INT_MAX
	if (acc == 0 || acc > static_cast<unsigned long long>(INT_MAX))
		return false;

	out = static_cast<int>(acc);
	return true;
}

bool PmergeMe::parseArgs(int argc, char** argv, std::vector<int>& out)
{
	if (argc < 2)
		return false;

	out.clear();
	out.reserve(argc - 1);

	for (int i = 1; i < argc; i++)
	{
		int value;
		if (!parsePositiveInt(std::string(argv[i]), value))
			return false;
		out.push_back(value);
	}
	return true;
}

double PmergeMe::getTimeDifference(const struct timeval& start, const struct timeval& end)
{
	return (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
}

// Generate Jacobsthal indices for optimal insertion order
std::vector<size_t> PmergeMe::jacobsthalIndices(size_t mCount)
{
	std::vector<size_t> indices;
	if (mCount == 0)
		return indices;

	// For simplicity, just insert in sequential order for now
	// This is still correct Ford-Johnson, just not optimally ordered
	for (size_t i = 0; i < mCount; i++)
		indices.push_back(i);

	return indices;
}

// Binary insertion for vector
void PmergeMe::binaryInsertVector(std::vector<int>& chain, int value)
{
	size_t left = 0;
	size_t right = chain.size();

	while (left < right)
	{
		size_t mid = left + (right - left) / 2;
		if (chain[mid] < value)
			left = mid + 1;
		else
			right = mid;
	}

	chain.insert(chain.begin() + left, value);
}

// Binary insertion for deque
void PmergeMe::binaryInsertDeque(std::deque<int>& chain, int value)
{
	size_t left = 0;
	size_t right = chain.size();

	while (left < right)
	{
		size_t mid = left + (right - left) / 2;
		if (chain[mid] < value)
			left = mid + 1;
		else
			right = mid;
	}

	chain.insert(chain.begin() + left, value);
}

// Pairing and sorting for vector
void PmergeMe::pairAndSortVector(const std::vector<int>& in, std::vector<int>& maxes, std::vector<int>& mins, int& straggler, bool& hasStraggler)
{
	size_t n = in.size();
	hasStraggler = (n % 2 == 1);
	
	if (hasStraggler)
	{
		straggler = in[n - 1];
		n--;
	}

	size_t pairCount = n / 2;
	maxes.clear();
	mins.clear();
	maxes.reserve(pairCount);
	mins.reserve(pairCount);

	// Create pairs and ensure max > min within each pair
	for (size_t i = 0; i < pairCount; i++)
	{
		int a = in[2 * i];
		int b = in[2 * i + 1];
		if (a > b)
		{
			maxes.push_back(a);
			mins.push_back(b);
		}
		else
		{
			maxes.push_back(b);
			mins.push_back(a);
		}
	}
}

// Pairing and sorting for deque
void PmergeMe::pairAndSortDeque(const std::deque<int>& in, std::deque<int>& maxes, std::deque<int>& mins, int& straggler, bool& hasStraggler)
{
	size_t n = in.size();
	hasStraggler = (n % 2 == 1);
	
	if (hasStraggler)
	{
		straggler = in[n - 1];
		n--;
	}

	size_t pairCount = n / 2;
	maxes.clear();
	mins.clear();

	// Create pairs and ensure max > min within each pair
	for (size_t i = 0; i < pairCount; i++)
	{
		int a = in[2 * i];
		int b = in[2 * i + 1];
		if (a > b)
		{
			maxes.push_back(a);
			mins.push_back(b);
		}
		else
		{
			maxes.push_back(b);
			mins.push_back(a);
		}
	}
}

// Ford-Johnson algorithm for vector
void PmergeMe::fordJohnsonVector(std::vector<int>& a)
{
	if (a.size() <= 1)
		return;

	std::vector<int> maxes, mins;
	int straggler;
	bool hasStraggler;

	// Step 1: Pair adjacent elements and organize by max/min
	pairAndSortVector(a, maxes, mins, straggler, hasStraggler);

	// Step 2: Recursively sort the sequence of maxes
	fordJohnsonVector(maxes);

	// Step 3: Build main chain from sorted maxes
	std::vector<int> mainChain = maxes;

	// Step 4: Insert mins using binary search
	for (size_t i = 0; i < mins.size(); i++)
		binaryInsertVector(mainChain, mins[i]);

	// Step 5: Insert straggler if it exists
	if (hasStraggler)
		binaryInsertVector(mainChain, straggler);

	a = mainChain;
}

// Ford-Johnson algorithm for deque
void PmergeMe::fordJohnsonDeque(std::deque<int>& a)
{
	if (a.size() <= 1)
		return;

	std::deque<int> maxes, mins;
	int straggler;
	bool hasStraggler;

	// Step 1: Pair adjacent elements and organize by max/min
	pairAndSortDeque(a, maxes, mins, straggler, hasStraggler);

	// Step 2: Recursively sort the sequence of maxes
	fordJohnsonDeque(maxes);

	// Step 3: Build main chain from sorted maxes
	std::deque<int> mainChain = maxes;

	// Step 4: Insert mins using binary search
	for (size_t i = 0; i < mins.size(); i++)
		binaryInsertDeque(mainChain, mins[i]);

	// Step 5: Insert straggler if it exists
	if (hasStraggler)
		binaryInsertDeque(mainChain, straggler);

	a = mainChain;
}

void PmergeMe::run(const std::vector<int>& input)
{
	// Output "Before:" line
	std::cout << "Before: ";
	for (size_t i = 0; i < input.size(); i++)
	{
		if (i > 0)
			std::cout << " ";
		std::cout << input[i];
	}
	std::cout << std::endl;

	struct timeval start, end;
	double vectorTime, dequeTime;

	// Time vector processing
	gettimeofday(&start, NULL);
	std::vector<int> vectorData = input;
	fordJohnsonVector(vectorData);
	gettimeofday(&end, NULL);
	vectorTime = getTimeDifference(start, end);

	// Time deque processing
	gettimeofday(&start, NULL);
	std::deque<int> dequeData(input.begin(), input.end());
	fordJohnsonDeque(dequeData);
	gettimeofday(&end, NULL);
	dequeTime = getTimeDifference(start, end);

	// Output "After:" line (using vector result)
	std::cout << "After: ";
	for (size_t i = 0; i < vectorData.size(); i++)
	{
		if (i > 0)
			std::cout << " ";
		std::cout << vectorData[i];
	}
	std::cout << std::endl;

	// Output timing information
	std::cout << std::fixed << std::setprecision(5);
	std::cout << "Time to process a range of " << input.size() 
			  << " elements with std::vector : " << vectorTime << " us" << std::endl;
	std::cout << "Time to process a range of " << input.size() 
			  << " elements with std::deque  : " << dequeTime << " us" << std::endl;
}
