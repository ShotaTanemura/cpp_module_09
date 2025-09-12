#include "BitcoinExchange.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

static bool parseInputLine(const std::string &line, std::string &date, std::string &valueStr) {
    // Check for exact format: "YYYY-MM-DD | value"
    size_t pipePos = line.find(" | ");
    if (pipePos == std::string::npos) {
        return false;
    }

    // Check there's only one pipe sequence
    if (line.find(" | ", pipePos + 3) != std::string::npos) {
        return false;
    }

    date = line.substr(0, pipePos);
    valueStr = line.substr(pipePos + 3);

    // Check for empty parts or extra whitespace
    if (date.empty() || valueStr.empty()) {
        return false;
    }

    // Check for leading/trailing whitespace
    if (date[0] == ' ' || date[date.length()-1] == ' ' ||
        valueStr[0] == ' ' || valueStr[valueStr.length()-1] == ' ') {
        return false;
    }

    return true;
}

static bool checkOverflow(long double value, long double rate, long double &result) {
    if (rate == 0) {
        result = 0;
        return true;
    }

    long double maxVal = std::numeric_limits<long double>::max();
    if (value > maxVal / rate) {
        return false;
    }

    result = value * rate;
    
    // Check result is finite
    if (result != result || result == std::numeric_limits<long double>::infinity()) {
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Error: could not open file." << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "Error: could not open file." << std::endl;
        return 1;
    }

    BitcoinExchange *exchange;
    try {
        exchange = new BitcoinExchange();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        inputFile.close();
        return 1;
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(inputFile, line)) {
        // Handle optional header
        if (firstLine) {
            firstLine = false;
            if (line == "date | value") {
                continue;
            }
        }

        std::string date, valueStr;
        if (!parseInputLine(line, date, valueStr)) {
            std::cout << "Error: bad input => " << line << std::endl;
            continue;
        }

        // Validate date
        if (!BitcoinExchange::isValidDate(date)) {
            std::cout << "Error: bad input => " << line << std::endl;
            continue;
        }

        // Parse and validate value
        long double value;
        if (!BitcoinExchange::isValidInputValue(valueStr, value)) {
            std::cout << "Error: bad input => " << line << std::endl;
            continue;
        }

        // Check value bounds
        if (value < 0) {
            std::cout << "Error: not a positive number." << std::endl;
            continue;
        }
        if (value > 1000) {
            std::cout << "Error: too large a number." << std::endl;
            continue;
        }

        // Check if rate is available
        if (!exchange->hasRateOnOrBefore(date)) {
            std::cout << "Error: no rate available for " << date << "." << std::endl;
            continue;
        }

        // Get rate and calculate product
        try {
            long double rate = exchange->rateOnOrBefore(date);
            long double result;
            
            if (!checkOverflow(value, rate, result)) {
                std::cout << "Error: multiplication overflow." << std::endl;
                continue;
            }

            // Output result
            std::cout << date << " => " << valueStr << " = " << result << std::endl;
            
        } catch (const std::exception &e) {
            std::cout << "Error: no rate available for " << date << "." << std::endl;
            continue;
        }
    }

    inputFile.close();
    delete exchange;
    return 0;
}
