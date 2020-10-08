#include "parse.h"

std::stringstream compile(const std::string& prog)
{
	std::stringstream op;
	op <<	"\tglobal main\n"
			"\textern ExitProcess\n"
			"\n"
			"section .text\n";

	return op;
}