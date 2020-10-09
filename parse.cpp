#include "parse.h"

ParseCursor::ParseCursor(const char* str)
	: cur(str) {}

std::stringstream compile(const std::string& prog)
{
	std::stringstream op;
	op <<	"\tglobal main\n"
			"\textern ExitProcess\n"
			"\n"
			"section .text\n";

	ParseCursor pc(prog.c_str());

	return op;
}