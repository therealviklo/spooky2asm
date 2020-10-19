#include "parseUtils.h"

bool validNameChar(char c) noexcept
{
	return isalnum(c) || c == '_';
}

std::string getFuncLabel(const std::string& func)
{
	if (func == "main") return func;
	return "_" + func;
}

bool peekcmp(const char* str, const char* cmp) noexcept
{
	while (*cmp != '\0')
	{
		if (*str != *cmp) return false;
		str++;
		cmp++;
	}
	return true;
}