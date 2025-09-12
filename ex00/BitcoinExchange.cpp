#include "BitcoinExchange.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <cstdlib>

BitcoinExchange::BitcoinExchange(const std::string &csvPath) {
    loadCsvDatabase(csvPath);
}

BitcoinExchange::~BitcoinExchange() {
}

BitcoinExchange::BitcoinExchange(const BitcoinExchange &other) : rates_(other.rates_) {
}

BitcoinExchange &BitcoinExchange::operator=(const BitcoinExchange &other) {
    if (this != &other) {
        rates_ = other.rates_;
    }
    return *this;
}

bool BitcoinExchange::hasRateOnOrBefore(const std::string &date) const {
    std::map<std::string, long double>::const_iterator it = rates_.lower_bound(date);
    if (it != rates_.end() && it->first == date) {
        return true;
    }
    return it != rates_.begin();
}

long double BitcoinExchange::rateOnOrBefore(const std::string &date) const {
    std::map<std::string, long double>::const_iterator it = rates_.lower_bound(date);
    if (it != rates_.end() && it->first == date) {
        return it->second;
    }
    if (it == rates_.begin()) {
        throw std::runtime_error("No rate available for " + date);
    }
    --it;
    return it->second;
}

void BitcoinExchange::loadCsvDatabase(const std::string &csvPath) {
    std::ifstream file(csvPath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Error: could not open database file.");
    }

    std::string line;
    bool firstLine = true;
    
    while (std::getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            if (line == "date,exchange_rate") {
                continue;
            }
            // If not header, process as data line
        }

        // Parse CSV line
        size_t commaPos = line.find(',');
        if (commaPos == std::string::npos) {
            std::cerr << "Error: bad database entry => " << line << std::endl;
            continue;
        }

        std::string dateStr = line.substr(0, commaPos);
        std::string rateStr = line.substr(commaPos + 1);

        // Check for extra whitespace or multiple commas
        if (dateStr.find(' ') != std::string::npos || 
            rateStr.find(' ') != std::string::npos ||
            rateStr.find(',') != std::string::npos) {
            std::cerr << "Error: bad database entry => " << line << std::endl;
            continue;
        }

        // Validate date
        if (!isValidDate(dateStr)) {
            std::cerr << "Error: bad database entry => " << line << std::endl;
            continue;
        }

        // Parse and validate rate
        long double rate;
        if (!isValidCsvRate(rateStr, rate)) {
            std::cerr << "Error: bad database entry => " << line << std::endl;
            continue;
        }

        // Check for duplicate dates
        if (rates_.find(dateStr) != rates_.end()) {
            std::cerr << "Error: bad database entry => " << line << std::endl;
            continue;
        }

        rates_[dateStr] = rate;
    }

    file.close();
    if (rates_.empty()) {
        throw std::runtime_error("Error: no valid entries in database.");
    }
}

bool BitcoinExchange::isValidDate(const std::string &date) {
    if (date.length() != 10) {
        return false;
    }
    
    // Check format: YYYY-MM-DD
    if (date[4] != '-' || date[7] != '-') {
        return false;
    }

    // Check all other characters are digits
    for (size_t i = 0; i < date.length(); ++i) {
        if (i == 4 || i == 7) continue;
        if (date[i] < '0' || date[i] > '9') {
            return false;
        }
    }

    int year, month, day;
    if (!BitcoinExchange::parseDateComponents(date, year, month, day)) {
        return false;
    }

    // Validate ranges
    if (year < 1 || month < 1 || month > 12 || day < 1) {
        return false;
    }

    int maxDays = getDaysInMonth(month, year);
    return day <= maxDays;
}

bool BitcoinExchange::parseDateComponents(const std::string &date, int &year, int &month, int &day) {
    if (date.length() != 10) {
        return false;
    }

    std::string yearStr = date.substr(0, 4);
    std::string monthStr = date.substr(5, 2);
    std::string dayStr = date.substr(8, 2);

    char *endptr;
    year = static_cast<int>(std::strtol(yearStr.c_str(), &endptr, 10));
    if (*endptr != '\0') return false;
    
    month = static_cast<int>(std::strtol(monthStr.c_str(), &endptr, 10));
    if (*endptr != '\0') return false;
    
    day = static_cast<int>(std::strtol(dayStr.c_str(), &endptr, 10));
    if (*endptr != '\0') return false;

    return true;
}

bool BitcoinExchange::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int BitcoinExchange::getDaysInMonth(int month, int year) {
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return isLeapYear(year) ? 29 : 28;
        default:
            return 0;
    }
}

bool BitcoinExchange::isValidCsvRate(const std::string &rateStr, long double &rate) {
    if (rateStr.empty()) {
        return false;
    }

    // Check for invalid characters and signs
    for (size_t i = 0; i < rateStr.length(); ++i) {
        char c = rateStr[i];
        if (c != '.' && (c < '0' || c > '9')) {
            return false;
        }
    }

    // Check for multiple decimal points
    size_t decimalCount = 0;
    for (size_t i = 0; i < rateStr.length(); ++i) {
        if (rateStr[i] == '.') {
            decimalCount++;
        }
    }
    if (decimalCount > 1) {
        return false;
    }

    char *endptr;
    rate = std::strtold(rateStr.c_str(), &endptr);
    
    // Check if entire string was consumed
    if (*endptr != '\0') {
        return false;
    }

    // Check for valid non-negative finite number
    if (rate < 0 || rate != rate || rate == std::numeric_limits<long double>::infinity()) {
        return false;
    }

    return true;
}

bool BitcoinExchange::isValidInputValue(const std::string &valueStr, long double &value) {
    if (valueStr.empty()) {
        return false;
    }

    // Check for invalid characters (allow minus sign at start)
    for (size_t i = 0; i < valueStr.length(); ++i) {
        char c = valueStr[i];
        if (c != '.' && (c < '0' || c > '9') && !(i == 0 && c == '-')) {
            return false;
        }
    }

    // Check for multiple decimal points
    size_t decimalCount = 0;
    for (size_t i = 0; i < valueStr.length(); ++i) {
        if (valueStr[i] == '.') {
            decimalCount++;
        }
    }
    if (decimalCount > 1) {
        return false;
    }

    char *endptr;
    value = std::strtold(valueStr.c_str(), &endptr);
    
    // Check if entire string was consumed
    if (*endptr != '\0') {
        return false;
    }

    // Check for valid finite number
    if (value != value || value == std::numeric_limits<long double>::infinity()) {
        return false;
    }

    return true;
}
