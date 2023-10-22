#include <string>
#include <windows.h>
#include "helpers.h"

std::string trim(const std::string& str) {
    // Returns index of first character not present in the arg
    // If it doesn't find matches it returns std::string::npos which is a sentinel constant (max value of size_t)
    size_t start = str.find_first_not_of(" \t");
    
    // Reverse of above to get end index
    size_t end = str.find_last_not_of(" \t");
    
    // Ternary, returns empty if no match or returns the str if no match (without spaces)
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}