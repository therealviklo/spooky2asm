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

void ParseCursor::skipTo(const char* pos)
{
	while (cur < pos) move();
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

void ParseCursor::skipWhitespace()
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
}

bool ParseCursor::tryParse(const char* cmpStr)
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
	if (cur > end) error("unexpected end of sequence");
	return true;
}

bool ParseCursor::tryParseWord(const char* cmpStr)
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
	if (cur > end) error("unexpected end of sequence");
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
	if (cur > end) error("unexpected end of sequence");
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

void evaluateExpression(ParseCursor pc, std::stringstream& op, LocalStack& localStack, Scope& scope)
{
	auto precedence = [](ParseCursor& c) -> int {
		if (c.tryParse("=")) return 3;
		if (c.tryParse("+")) return 2;
		if (c.tryParse("-")) return 2;
		if (c.tryParse("%")) return 1;
		if (c.tryParse("*")) return 1;
		if (c.tryParse("/")) return 1;
		return 0;
	};

	ParseCursor splitter = pc;
	int paramLevel = 0;
	const char* currOp = nullptr;
	const char* currOpEnd = nullptr;
	int currPrecedence = 0;
	while (!splitter.atEnd())
	{
		if (*splitter == '(') paramLevel++;
		else if (*splitter == ')') paramLevel--;
		if (!paramLevel)
		{
			const char* const opBegin = splitter;
			const int prec = precedence(splitter);
			if (prec > currPrecedence)
			{
				currPrecedence = prec;
				currOp = opBegin;
				currOpEnd = splitter;
			}
			if (prec == 0) splitter.move();
		}
		else
		{
			splitter.move();
		}
	}

	if (currOp)
	{
		switch (*currOp)
		{
			case '=':
			{
				std::stringstream nameSS;
				pc.skipWhitespace();
				while (validNameChar(*pc))
				{
					nameSS << *pc;
					pc.move();
				}
				const std::string name = nameSS.str();
				if (scope.has(name))
				{
					pc.skipWhitespace();
					if (!pc.tryParse("=")) pc.error("invalid syntax, expected '='");
				}
				else
				{
					pc.skipWhitespace();
					if (!pc.tryParse(":")) pc.error("invalid syntax, expected ':'");
					pc.skipWhitespace();
					const std::string typeName = pc.readIdentifier();
					scope.add(name, {typeName, localStack.getQword()});
					pc.skipWhitespace();
					if (!pc.tryParse("=")) pc.error("invalid syntax, expected '=' (2)");
				}

				evaluateExpression(pc, op, localStack, scope);

				const std::string loc = scope.get(name).location;
				op << "\tmov " << loc << ", rax\n";
			}
			return;
			case '+':
			{
				ParseCursor firstPc = pc;
				firstPc.skipWhitespace();
				if (firstPc.tryParse("+"))
				{
					op << "\txor rax, rax\n";
				}
				else
				{
					firstPc.setEnd(currOp);
					evaluateExpression(firstPc, op, localStack, scope);
				}

				std::string tmp = localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				ParseCursor secondPc = pc;
				secondPc.skipTo(currOpEnd);
				secondPc.setEnd(pc.getEnd());
				evaluateExpression(secondPc, op, localStack, scope);

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\tadd rax, rcx\n";
			}
			return;
			case '-':
			{
				ParseCursor firstPc = pc;
				firstPc.skipWhitespace();
				if (firstPc.tryParse("-"))
				{
					op << "\txor rax, rax\n";
				}
				else
				{
					firstPc.setEnd(currOp);
					evaluateExpression(firstPc, op, localStack, scope);
				}

				std::string tmp = localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				ParseCursor secondPc = pc;
				secondPc.skipTo(currOpEnd);
				secondPc.setEnd(pc.getEnd());
				evaluateExpression(secondPc, op, localStack, scope);

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\tsub rax, rcx\n";
			}
			return;
			case '%':
			{
				ParseCursor firstPc = pc;
				firstPc.setEnd(currOp);
				evaluateExpression(firstPc, op, localStack, scope);

				std::string tmp = localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				ParseCursor secondPc = pc;
				secondPc.skipTo(currOpEnd);
				secondPc.setEnd(pc.getEnd());
				evaluateExpression(secondPc, op, localStack, scope);

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\txor rdx, rdx\n"
						"\tidiv rcx\n"
						"\tmov rax, rdx\n";
			}
			return;
			case '*':
			{
				ParseCursor firstPc = pc;
				firstPc.setEnd(currOp);
				evaluateExpression(firstPc, op, localStack, scope);

				std::string tmp = localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				ParseCursor secondPc = pc;
				secondPc.skipTo(currOpEnd);
				secondPc.setEnd(pc.getEnd());
				evaluateExpression(secondPc, op, localStack, scope);

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\timul rax, rcx\n";
			}
			return;
			case '/':
			{
				ParseCursor firstPc = pc;
				firstPc.setEnd(currOp);
				evaluateExpression(firstPc, op, localStack, scope);

				std::string tmp = localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				ParseCursor secondPc = pc;
				secondPc.skipTo(currOpEnd);
				secondPc.setEnd(pc.getEnd());
				evaluateExpression(secondPc, op, localStack, scope);

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\txor rdx, rdx\n"
						"\tidiv rcx\n";
			}
			return;
		}
	}
	else
	{
		pc.skipWhitespace();
		if (isdigit(*pc))
		{
			op <<	"\tmov rax, " << std::strtoll(pc, nullptr, 0) << "\n";
		}
		else if (*pc == '(')
		{
			ParseCursor paramPc = pc;
			paramPc.move();
			int closeParamLevel = 0;
			ParseCursor closeParamPc = paramPc;
			while (!closeParamLevel && *closeParamPc != ')')
			{
				if (*closeParamPc == '(') closeParamLevel++;
				else if (*closeParamPc == ')') closeParamLevel--;
				closeParamPc.move();
			}
			paramPc.setEnd(closeParamPc);
			evaluateExpression(paramPc, op, localStack, scope);
		}
		else
		{
			std::stringstream ss;
			while (validNameChar(*pc))
			{
				ss << *pc;
				pc.move();
			}
			const std::string loc = scope.get(ss.str()).location;
			pc.skipWhitespace();
			if (!pc.atEnd()) pc.error("invalid syntax");
			op << "\tmov rax, " << loc << "\n";
		}
	}
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
		ParseCursor exprCur = pc;
		while (*pc != ';') pc.move();
		exprCur.setEnd(pc);
		pc.move();
		evaluateExpression(exprCur, body, localStack, scope);
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