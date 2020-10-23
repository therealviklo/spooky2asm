#include "parseCursor.h"

bool ParseCursor::atEnd()
{
	skipWhitespace();
	return cur >= end;
}

void ParseCursor::skipWhitespace()
{
	auto atComment = [&]() -> bool {
		return *cur == '/' && *(cur + 1) == '/';
	};

	while (true)
	{
		while (isspace(*cur))
		{
			if (cur >= end) break;
			if (*cur == '\n')
			{
				x = 1;
				y++;
			}
			else
			{
				x++;
			}
			cur++;
		}

		if (atComment())
		{
			while (*cur != '\n' && *cur != '\0') move();
			continue;
		}
		break;
	}
}

ParseCursor::ParseCursor(const char* str)
	: cur(str),
	  end((const char *)~0ull),
	  x(1),
	  y(1)
{
	if (tryParse("\ufeff")) // BOM
	{
		x = 1;
		y = 1;
	}
}

void ParseCursor::skipParen()
{
	size_t paramLevel = 0;
	while (paramLevel || !tryParse(")"))
	{
		if (*cur == '(') paramLevel++;
		else if (*cur == ')') paramLevel--;
		move();
	}
}

void ParseCursor::skipBlock()
{
	size_t paramLevel = 0;
	while (paramLevel || !tryParse("}"))
	{
		if (*cur == '{') paramLevel++;
		else if (*cur == '}') paramLevel--;
		move();
	}
}

void ParseCursor::skipNameOrNumber()
{
	skipWhitespace();

	if (!validNameChar(*cur)) error("name expected");
	do
	{
		if (*cur == '\n')
		{
			x = 1;
			y++;
		}
		else
		{
			x++;
		}
		cur++;
	} while (validNameChar(*cur));
	if (cur > end) error("unexpected end of sequence");
}

void ParseCursor::move()
{
	if (*cur == '\0') throw EndOfFileException("unexpected end of file");
	if (*cur == '\n')
	{
		x = 1;
		y++;
	}
	else
	{
		x++;
	}
	cur++;
	if (cur > end) error("unexpected end of sequence");
}

void ParseCursor::move(size_t num)
{
	for (size_t i = 0; i < num; i++)
	{
		if (*cur == '\0') throw EndOfFileException("unexpected end of file");
		if (*cur == '\n')
		{
			x = 1;
			y++;
		}
		else
		{
			x++;
		}
		cur++;
	}
	if (cur > end) error("unexpected end of sequence");
}

bool ParseCursor::tryParse(const char* cmpStr)
{
	skipWhitespace();

	const char* peekStr = cur;
	size_t lx = x;
	size_t ly = y;
	while (cmpStr[0] != '\0')
	{
		if (peekStr[0] != cmpStr[0]) return false;
		if (*peekStr == '\n')
		{
			lx = 1;
			ly++;
		}
		else
		{
			lx++;
		}
		peekStr++;
		cmpStr++;
	}
	cur = peekStr;
	x = lx;
	y = ly;
	if (cur > end) error("unexpected end of sequence");
	return true;
}

bool ParseCursor::tryParseWord(const char* cmpStr)
{
	skipWhitespace();

	const char* peekStr = cur;
	size_t lx = x;
	size_t ly = y;
	while (cmpStr[0] != '\0')
	{
		if (peekStr[0] != cmpStr[0]) return false;
		if (*peekStr == '\n')
		{
			lx = 1;
			ly++;
		}
		else
		{
			lx++;
		}
		peekStr++;
		cmpStr++;
	}
	if (validNameChar(peekStr[0])) return false;
	cur = peekStr;
	x = lx;
	y = ly;
	if (cur > end) error("unexpected end of sequence");
	return true;
}

std::string ParseCursor::readIdentifier()
{
	skipWhitespace();

	std::stringstream ss;
	if (!validNameChar(*cur)) error("name expected");
	if (isdigit(*cur)) error("names cannot start with a digit");
	do
	{
		ss << *cur;
		if (*cur == '\n')
		{
			x = 1;
			y++;
		}
		else
		{
			x++;
		}
		cur++;
	} while (validNameChar(*cur));
	if (cur > end) error("unexpected end of sequence");
	return ss.str();
}

void ParseCursor::error(const char* msg)
{
	throw InvalidSyntaxException(msg, x, y);
}

char ParseCursor::operator*()
{
	skipWhitespace();
	return *cur;
}

const char* ParseCursor::str()
{
	skipWhitespace();
	return cur;
}