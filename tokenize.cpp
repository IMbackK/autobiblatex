#include "tokenize.h"

std::vector<std::string> tokenize(const std::string& str, const char delim, const char ignBracketStart, const char ignBracketEnd)
{
	std::stringstream ss(str);
	std::vector<std::string> tokens;

	std::string token;
	size_t bracketCounter = 0;
	for(char ch : str)
	{
		if(ch == delim && bracketCounter == 0)
		{
			tokens.push_back(token);
			token.clear();
		}
		else
		{
			token.push_back(ch);
		}

		if(ignBracketStart == ch)
			++bracketCounter;
		else if(ignBracketEnd == ch)
			--bracketCounter;
	}
	if(bracketCounter == 0)
		tokens.push_back(token);
	return tokens;
}
