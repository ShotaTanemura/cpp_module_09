#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <map>
#include <string>
#include <stdexcept>

class BitcoinExchange {
public:
    explicit BitcoinExchange(const std::string &csvPath = "data.csv");
    ~BitcoinExchange();
    BitcoinExchange(const BitcoinExchange &other);
    BitcoinExchange &operator=(const BitcoinExchange &other);

    bool hasRateOnOrBefore(const std::string &date) const;
    long double rateOnOrBefore(const std::string &date) const;

    // Static utility functions for date and number validation
    static bool isValidDate(const std::string &date);
    static bool isValidCsvRate(const std::string &rateStr, long double &rate);
    static bool isValidInputValue(const std::string &valueStr, long double &value);
    static bool isLeapYear(int year);
    static int getDaysInMonth(int month, int year);
    static bool parseDateComponents(const std::string &date, int &year, int &month, int &day);

private:
    std::map<std::string, long double> rates_;
    
    void loadCsvDatabase(const std::string &csvPath);
};

#endif
