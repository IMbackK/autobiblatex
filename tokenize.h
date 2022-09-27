#pragma once
#include <string>
#include <vector>
#include <sstream>

std::vector<std::string> tokenize(const std::string& str, const char delim = ' ', const char ignBracketStart = '\0', const char ignBracketEnd = '\0');
