#pragma once
#include <string>
#include <sstream>
#include "parseExceptions.h"
#include "parseUtils.h"

class ParseCursor
{
private:
	const char* cur;
	const char* end;
	size_t x;
	size_t y;

	void skipWhitespace();
public:
	ParseCursor(const char* str);

	void setEnd(const ParseCursor& end) noexcept { this->end = end.cur; }
	constexpr bool atEnd() const noexcept { return cur >= end; }

	void skipParen();
	void skipNameOrNumber();

	void move();
	void move(size_t num);

	bool tryParse(const char* cmpStr);
	bool tryParseWord(const char* cmpStr);
	std::string readIdentifier();

	void error(const char* msg);

	char operator*();
	const char* str();
};