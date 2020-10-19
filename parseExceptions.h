#pragma once
#include <stdexcept>

class EndOfFileException : public std::runtime_error
{
public:
	EndOfFileException(const char* msg) : std::runtime_error(msg) {}
};

class InvalidSyntaxException : public std::runtime_error
{
public:
	size_t x;
	size_t y;

	InvalidSyntaxException(const char* msg, size_t x, size_t y) : std::runtime_error(msg), x(x), y(y) {}
};