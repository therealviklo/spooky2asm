#pragma once
#include <string>
#include <sstream>

class ParseCursor
{
private:
	const char* cur;
public:
	ParseCursor(const char* str);
};

std::stringstream compile(const std::string& prog);