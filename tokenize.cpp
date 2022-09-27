/**
* autobiblatex
* Copyright (C) 2022 Carl Klemm
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 3 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
*/

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
