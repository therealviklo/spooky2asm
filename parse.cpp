#include "parse.h"

bool validNameChar(char c) noexcept
{
	return isalnum(c) || c == '_';
}

std::string getFuncLabel(const std::string& func)
{
	if (func == "main") return func;
	return "_" + func;
}

ParseCursor::ParseCursor(const char* str)
	: cur(str),
	  x(1),
	  y(1)
{
	if (tryParse("\ufeff")) // BOM
	{
		x = 1;
		y = 1;
	}
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
}

void ParseCursor::skipWhitespace() noexcept
{
	while (isspace(*cur))
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
	}
}

bool ParseCursor::tryParse(const char* cmpStr) noexcept
{
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
	return true;
}

bool ParseCursor::tryParseWord(const char* cmpStr) noexcept
{
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
	return true;
}

std::string ParseCursor::readIdentifier()
{
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
	return ss.str();
}

inline void ParseCursor::error(const char* msg)
{
	throw InvalidSyntaxException(msg, x, y);
}

std::string LocalStack::getQword()
{
	stackSize += 8;
	std::stringstream ss;
	ss << "qword [rbp - ";
	ss << stackSize;
	ss << "]";
	return ss.str();
}

std::stringstream LocalStack::getStackInit()
{
	std::stringstream ss;
	ss <<	"\tpush rbp\n"
			"\tmov rbp, rsp\n"
			"\tsub rsp, ";
	ss << stackSize;
	ss << '\n';
	return ss;
}

bool Scope::has(std::string var) const
{
	for (auto i = scopes.rbegin(); i != scopes.rend(); i++)
	{
		if (i->count(var)) return true;
	}
	return false;
}

Variable& Scope::get(std::string var)
{
	for (auto i = scopes.rbegin(); i != scopes.rend(); i++)
	{
		if (i->count(var)) return i->at(var);
	}
	throw InvalidSyntaxException(("undefined name: " + var).c_str(), 0, 0);
}

void Scope::add(std::string varName, Variable var)
{
	scopes[scopes.size() - 1].insert({std::move(varName), std::move(var)});
}

void Scope::addScope()
{
	scopes.emplace_back();
}

void Scope::removeScope() noexcept
{
	if (scopes.size()) scopes.pop_back();
}

LocalScope::LocalScope(Scope& scope)
	: scope(scope)
{
	scope.addScope();
}

LocalScope::~LocalScope()
{
	scope.removeScope();
}

void evaluateExpression(ParseCursor& pc, std::stringstream& op, LocalStack& localStack, Scope& scope)
{

}

void generateFunction(ParseCursor& pc, std::stringstream& op)
{
	std::string funcName = pc.readIdentifier();

	LocalStack localStack;
	Scope scope;
	LocalScope localScope(scope);
	
	pc.skipWhitespace();
	if (!pc.tryParse("(")) pc.error("expected '('");
	size_t paramOffset = 0;
	pc.skipWhitespace();
	if (!pc.tryParse(")"))
	{
		while (true)
		{
			std::string varName = pc.readIdentifier();
			if (scope.has(varName)) pc.error("redefinition of variable");
			
			pc.skipWhitespace();
			if (!pc.tryParse(":")) pc.error("expected ':'");
		
			pc.skipWhitespace();
			std::string typeName = pc.readIdentifier();

			std::stringstream ss;
			ss << "qword [rbp + ";
			ss << (16 + paramOffset);
			ss << "]";
			paramOffset += 8;
			
			scope.add(std::move(varName), {std::move(typeName), ss.str()});
			
			pc.skipWhitespace();

			if (pc.tryParse(","))
			{
				pc.skipWhitespace();
			}
			else if (pc.tryParse(")"))
			{
				break;
			}
			else
			{
				pc.error("unexpected character");
			}
		}
	}

	pc.skipWhitespace();
	if (!pc.tryParse("{")) pc.error("expected '{'");

	std::stringstream body;
	pc.skipWhitespace();
	while (!pc.tryParse("}"))
	{
		evaluateExpression(pc, body, localStack, scope);
		pc.skipWhitespace();
	}

	op << getFuncLabel(funcName) << ":\n";
	op << localStack.getStackInit().str();
	op << body.str();
	op <<	".ret:\n"
			"\tmov rsp, rbp\n"
			"\tpop rbp\n";
	if (funcName == "main")
	{
		op <<	"\txor rcx, rcx\n"
				"\tcall ExitProcess\n\n";
	}
	else
	{
		op << "\tret\n\n";
	}
}

std::stringstream compile(const std::string& prog)
{
	std::stringstream op;
	op <<	"\tglobal main\n"
			"\textern ExitProcess\n"
			"\n"
			"section .text\n";

	ParseCursor pc(prog.c_str());

	while (true)
	{
		pc.skipWhitespace();
		if (*pc == '\0') break;
		if (pc.tryParseWord("func"))
		{
			pc.skipWhitespace();
			generateFunction(pc, op);
		}
		else
		{
			pc.error("unexpected symbol");
		}
	}

	return op;
}