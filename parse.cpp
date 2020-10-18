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
			while (*cur != '\n') move();
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

inline void ParseCursor::error(const char* msg)
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

struct PrecedenceReturn {
	ExprOp op;
	int prec;
};
PrecedenceReturn precedence(ParseCursor& c)
{
	if (c.tryParse("==")) return {EO_EQ, 4}; // Den här är lägre än = men måste kollas innan

	if (c.tryParse("=")) return {EO_ASSIGN, 5};

	if (c.tryParse(">=")) return {EO_GE, 3};
	if (c.tryParse(">")) return {EO_GT, 3};
	if (c.tryParse("<=")) return {EO_LE, 3};
	if (c.tryParse("<")) return {EO_LT, 3};

	if (c.tryParse("+")) return {EO_ADD, 2};
	if (c.tryParse("-")) return {EO_SUB, 2};

	if (c.tryParse("%")) return {EO_MOD, 1};
	if (c.tryParse("*")) return {EO_MUL, 1};
	if (c.tryParse("/")) return {EO_DIV, 1};

	return {EO_NOOP, 0};
}

std::string Parser::evaluateExpression(ParseCursor& pc, std::stringstream& op, FunctionData& fd)
{
	struct Splitter {
		ExprOp op;
		int currPrecedence;
		ParseCursor& pc;
		ParseCursor first;
		ParseCursor second;

		void skipOperand()
		{
			if (pc.tryParse("("))
			{
				pc.skipParen();
			}
			else if (validNameChar(*pc))
			{
				do
				{
					pc.move();
				} while (validNameChar(*pc));
			}
			else
			{
				pc.error("unexpected character");
			}
		}

		// Return: vid slutet eller ej
		bool parseOp()
		{
			if (pc.atEnd()) return true;
			ParseCursor opStart = pc;
			auto precRet = precedence(pc);
			if (precRet.prec > currPrecedence)
			{
				first.setEnd(opStart);
				second = pc;
				op = precRet.op;
			}
			else if (precRet.prec == 0) return true;
			return false;
		}

		Splitter(ParseCursor& pc)
			: op(EO_NOOP),
			  currPrecedence(0),
			  pc(pc),
			  first(pc),
			  second(pc)
		{
			if (*pc == '+' || *pc == '-')
			{
				parseOp();
			}
			do
			{
				skipOperand();
			} while (!parseOp());
		}
	} splitter(pc);

		switch (splitter.op)
		{
			case EO_EQ:
			{
				if (evaluateExpression(splitter.first, op, fd).empty())
					pc.error("expression does not return value");

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\tsub rcx, rax\n"
						"\txor rax, rax\n"
						"\tcmp rcx, 0\n"
						"\tsete al\n";
			}
			return "Boolean";
			case EO_GE:
			{
				if (evaluateExpression(splitter.first, op, fd).empty())
					pc.error("expression does not return value");

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, " << tmp << "\n"
						"\tsub rcx, rax\n"
						"\txor rax, rax\n"
						"\tcmp rcx, 0\n"
						"\tsetge al\n";
			}
			return "Boolean";
			case EO_LE:
			{
				if (evaluateExpression(splitter.first, op, fd).empty())
					pc.error("expression does not return value");

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, " << tmp << "\n"
						"\tsub rcx, rax\n"
						"\txor rax, rax\n"
						"\tcmp rcx, 0\n"
						"\tsetle al\n";
			}
			return "Boolean";
			case EO_ASSIGN:
			{
				std::stringstream nameSS;
				while (validNameChar(*pc))
				{
					nameSS << *pc;
					pc.move();
				}
				const std::string name = nameSS.str();
				if (scope.has(name))
				{
					if (!pc.tryParse("=")) pc.error("invalid syntax, expected '='");
				}
				else
				{
					if (functions.count(name)) pc.error("variable name is already a function");
					if (!pc.tryParse(":")) pc.error("invalid syntax, expected ':'");
					const std::string typeName = pc.readIdentifier();
					scope.add(name, {typeName, fd.localStack.getQword()});
					if (!pc.tryParse("=")) pc.error("invalid syntax, expected '=' (2)");
				}

				if (evaluateExpression(pc, op, fd).empty())
					pc.error("expression does not return value");

				const std::string loc = scope.get(name).location;
				op << "\tmov " << loc << ", rax\n";

				return scope.get(name).type;
			}
			break; // Borde inte komma hit då det är en return ovan.
			case EO_GT:
			{
				if (evaluateExpression(splitter.first, op, fd).empty())
					pc.error("expression does not return value");

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, " << tmp << "\n"
						"\tsub rcx, rax\n"
						"\txor rax, rax\n"
						"\tcmp rcx, 0\n"
						"\tsetg al\n";
			}
			return "Boolean";
			case EO_LT:
			{
				if (evaluateExpression(splitter.first, op, fd).empty())
					pc.error("expression does not return value");

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, " << tmp << "\n"
						"\tsub rcx, rax\n"
						"\txor rax, rax\n"
						"\tcmp rcx, 0\n"
						"\tsetl al\n";
			}
			return "Boolean";
			case EO_ADD:
			{
				if (*splitter.first == '+')
				{
					op << "\txor rax, rax\n";
				}
				else
				{
					if (evaluateExpression(splitter.first, op, fd).empty())
						pc.error("expression does not return value");
				}

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\tadd rax, rcx\n";
			}
			return "Int";
			case EO_SUB:
			{
				if (*splitter.first == '-')
				{
					op << "\txor rax, rax\n";
				}
				else
				{
					if (evaluateExpression(splitter.first, op, fd).empty())
						pc.error("expression does not return value");
				}

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\tsub rax, rcx\n";
			}
			return "Int";
			case EO_MOD:
			{
				if (evaluateExpression(splitter.first, op, fd).empty())
					pc.error("expression does not return value");

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\txor rdx, rdx\n"
						"\tidiv rcx\n"
						"\tmov rax, rdx\n";
			}
			return "Int";
			case EO_MUL:
			{
				if (evaluateExpression(splitter.first, op, fd).empty())
					pc.error("expression does not return value");

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\timul rax, rcx\n";
			}
			return "Int";
			case EO_DIV:
			{
				if (evaluateExpression(splitter.first, op, fd).empty())
					pc.error("expression does not return value");

				std::string tmp = fd.localStack.getQword();
				op << "\tmov " << tmp << ", rax\n";

				if (evaluateExpression(splitter.second, op, fd).empty())
					pc.error("expression does not return value");

				op <<	"\tmov rcx, rax\n"
						"\tmov rax, " << tmp << "\n"
						"\txor rdx, rdx\n"
						"\tidiv rcx\n";
			}
			return "Int";
			case EO_NOOP:
			{
				if (isdigit(*splitter.first))
				{
					op <<	"\tmov rax, " << std::strtoll(splitter.first.str(), nullptr, 0) << "\n";

					return "Int";
				}
				else if (*splitter.first == '(')
				{
					ParseCursor paramPc = splitter.first;
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
					return evaluateExpression(paramPc, op, fd);
				}
				else
				{
					const std::string id = splitter.first.readIdentifier();

					if (scope.has(id))
					{
						const std::string loc = scope.get(id).location;
						if (!splitter.first.atEnd()) splitter.first.error("invalid syntax");
						op << "\tmov rax, " << loc << "\n";

						return scope.get(id).type;
					}
					else if (functions.count(id)) // Kör en funktion
					{
						// Hämta namnet som ska användas i assemblyn.
						const std::string funcLabel = getFuncLabel(id);
						
						// Skippa till och över parantesen.
						if (!splitter.first.tryParse("(")) splitter.first.error("expected '('");

						auto& func = functions.at(id);

						// Gör utrymme för argumenten.
						op << "\tsub rsp, " << (8 * func.argTypes.size()) << "\n";

						// Läs in och evaluera uttrycken och flytta dem till dit de ska.
						size_t currParamOffset = 0;
						while (!splitter.first.tryParse(")"))
						{
							// Skapa ParseCursorn för uttrycket.
							ParseCursor argCur = splitter.first;
							// Flytta splitter.first till kommatecknet eller parentesen. Ignorera allt som är i parenteser.
							int argParamLevel = 0;
							while (argParamLevel || (*splitter.first != ',' && *splitter.first != ')'))
							{
								if (*splitter.first == '(') argParamLevel++;
								else if (*splitter.first == ')') argParamLevel--;
								splitter.first.move();
							}
							// Sätt kommatecknet eller parentesen som slutet på argCur.
							argCur.setEnd(splitter.first);
							// Evaluera.
							if (evaluateExpression(argCur, op, fd).empty())
								splitter.first.error("expression does not return value");
							// Flytta till stacken.
							op << "\tmov qword [rsp + " << currParamOffset << "], rax\n";

							// Nästa
							currParamOffset += 8;
							if (splitter.first.tryParse(","))
							{
								if (*splitter.first == ')') splitter.first.error("expected expression");
							}
						}

						if (currParamOffset != 8 * func.argTypes.size())
						{
							splitter.first.error("wrong number of arguments");
						}

						// Kör och rensa upp.
						op <<	"\tcall " << funcLabel << "\n"
								"\tadd rsp, " << (8 * func.argTypes.size()) << "\n";

						return func.retType;
					}
					else
					{
						splitter.first.error("unknown identifier");
					}
				}
			}
			break;
		}
	return "";
}

void Parser::generateStatement(std::stringstream& op, FunctionData& fd)
{
	if (pc.tryParseWord("return"))
	{
		if (fd.retType.empty() && !pc.tryParse(";")) pc.error("expected ';'");
		else
		{
			ParseCursor exprCur = pc;
			while (*pc != ';') pc.move();
			exprCur.setEnd(pc);
			pc.move();
			if (evaluateExpression(exprCur, op, fd).empty())
				pc.error("expression does not return value");
		}
		op << "\tjmp .ret\n";
	}
	else if (pc.tryParseWord("if"))
	{
		const auto ifNum = labelManager.getIfNum();
		
		{
			LocalScope localScope(scope);

			if (!pc.tryParse("(")) pc.error("expected '('");
			ParseCursor exprCur = pc;
			size_t paramLevel = 0;
			while (paramLevel || *pc != ')')
			{
				if (*pc == '(') paramLevel++;
				else if (*pc == ')') paramLevel--;
				pc.move();
			}
			exprCur.setEnd(pc);
			pc.move();
			if (evaluateExpression(exprCur, op, fd).empty())
				pc.error("expression does not return value");

			op <<	"\tcmp rax, 0\n"
					"\tje .iff" << ifNum << "\n";
			
			if (pc.tryParse("{"))
			{
				generateBlock(op, fd);
			}
			else
			{
				generateStatement(op, fd);
			}
		}

		{
			LocalScope localScope(scope);

			if (pc.tryParseWord("else"))
			{
				op <<	"\tjmp .ife" << ifNum << "\n"
						".iff" << ifNum << ":\n";

				if (pc.tryParse("{"))
				{
					generateBlock(op, fd);
				}
				else
				{
					generateStatement(op, fd);
				}

				op << ".ife" << ifNum << ":\n";
			}
			else
			{
				op << ".iff" << ifNum << ":\n";
			}
		}
	}
	else
	{
		if (!pc.tryParse(";"))
		{
			ParseCursor exprCur = pc;
			while (*pc != ';') pc.move();
			exprCur.setEnd(pc);
			pc.move();
			evaluateExpression(exprCur, op, fd);
		}
	}
}

void Parser::generateBlock(std::stringstream& op, FunctionData& fd)
{
	LocalScope localScope(scope);
	while (!pc.tryParse("}"))
	{
		generateStatement(op, fd);
	}
}

void Parser::generateFunction(std::stringstream& op)
{
	std::string funcName = pc.readIdentifier();
	if (functions.count(funcName)) pc.error("function redefinition");
	std::vector<std::string> argTypes;

	FunctionData fd;
	LocalScope localScope(scope);

	if (!pc.tryParse("(")) pc.error("expected '('");
	size_t paramOffset = 0;
	if (!pc.tryParse(")"))
	{
		while (true)
		{
			std::string varName = pc.readIdentifier();
			if (scope.has(varName)) pc.error("redefinition of variable");
			if (functions.count(varName)) pc.error("variable name is already a function");
			
			if (!pc.tryParse(":")) pc.error("expected ':'");
		
			std::string typeName = pc.readIdentifier();
			argTypes.push_back(typeName);

			std::stringstream ss;
			ss << "qword [rbp + ";
			ss << (16 + paramOffset);
			ss << "]";
			paramOffset += 8;
			
			scope.add(std::move(varName), {std::move(typeName), ss.str()});
			
			if (!pc.tryParse(","))
			{
				if (pc.tryParse(")"))
				{
					break;
				}
				else
				{
					pc.error("unexpected character");
				}
			}
		}
	}

	if (pc.tryParse("->"))
	{
		fd.retType = pc.readIdentifier();
	}

	functions.insert({funcName, {std::move(argTypes), fd.retType}});

	if (!pc.tryParse("{")) pc.error("expected '{'");

	std::stringstream body;
	generateBlock(body, fd);

	op << getFuncLabel(funcName) << ":\n";
	op << fd.localStack.getStackInit().str();
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

void Parser::generateExtern(std::stringstream& op)
{
	std::string funcName = pc.readIdentifier();
	std::vector<std::string> argTypes;
	std::string retType;

	if (!pc.tryParse("(")) pc.error("expected '('");
	
	if (!pc.tryParse(")"))
	{
		while (true)
		{
			std::string varName = pc.readIdentifier();
			if (functions.count(varName)) pc.error("variable name is already a function");
			
			if (!pc.tryParse(":")) pc.error("expected ':'");
		
			std::string typeName = pc.readIdentifier();
			argTypes.push_back(typeName);
			
			if (!pc.tryParse(","))
			{
				if (pc.tryParse(")"))
				{
					break;
				}
				else
				{
					pc.error("unexpected character");
				}
			}
		}
	}

	if (pc.tryParse("->"))
	{
		retType = pc.readIdentifier();
	}

	auto checkTypes = [&](std::vector<std::string> typeKey) -> bool {
		if (argTypes.size() != typeKey.size()) return false;
		for (size_t i = 0; i < typeKey.size(); i++)
		{
			if (argTypes[i] != typeKey[i]) return false;
		}
		return true;
	};

	if (funcName == "print")
	{
		if (!checkTypes({"Int"})) pc.error("invalid extern arguments");
		if (!retType.empty()) pc.error("invalid extern return type");
		op <<	"_print:\n"
				"\tpush rbp\n"
				"\tmov rbp, rsp\n"
				"\txor rdx, rdx\n"
				"\tmov rax, rsp\n"
				"\tmov rcx, 16\n"
				"\tdiv rcx\n"
				"\tsub rsp, rdx\n"
				"\tsub rsp, 32\n"
				"\tmov rcx, qword [rbp + 16]\n"
				"\tcall putchar\n"
				"\tmov rsp, rbp\n"
				"\tpop rbp\n"
				"\tret\n"
				"\n";
	}
	else if (funcName == "printInt")
	{
		if (!checkTypes({"Int"})) pc.error("invalid extern arguments");
		if (!retType.empty()) pc.error("invalid extern return type");
		op <<	"_printInt:\n"
				"\tpush rbp\n"
				"\tmov rbp, rsp\n"
				"\txor rdx, rdx\n"
				"\tmov rax, rsp\n"
				"\tmov r8, 16\n"
				"\tdiv r8\n"
				"\tsub rsp, rdx\n"
				"\tmov rax, qword [rbp + 16]\n"
				"\tcmp rax, 0\n"
				"\tje .zero\n"
				"\tjmp .nozero\n"
				".zero:\n"
				"\tmov rcx, 48\n"
				"\tsub rsp, 40\n"
				"\tcall putchar\n"
				"\tjmp .ret1\n"
				".nozero:\n"
				"\tjl .neg\n"
				"\tjmp .noneg\n"
				".neg:\n"
				"\tmov rcx, 45\n"
				"\tpush rax\n"
				"\tsub rsp, 32\n"
				"\tcall putchar\n"
				"\tadd rsp, 32\n"
				"\tpop rax\n"
				"\tneg rax\n"
				".noneg:\n"
				"\tsub rsp, 8\n"
				"\tcall .loop\n"
				".ret1:\n"
				"\tmov rsp, rbp\n"
				"\tpop rbp\n"
				"\tret\n"
				".loop:\n"
				"\tpush rbp\n"
				"\tmov rbp, rsp\n"
				"\tcmp rax, 0\n"
				"\tje .ret2\n"
				"\txor rdx, rdx\n"
				"\tmov rcx, 10\n"
				"\tdiv rcx\n"
				"\tpush rdx\n"
				"\tcall .loop\n"
				"\tpop rdx\n"
				"\tmov rcx, rdx\n"
				"\tadd rcx, 48\n"
				"\tsub rsp, 40\n"
				"\tcall putchar\n"
				".ret2:\n"
				"\tmov rsp, rbp\n"
				"\tpop rbp\n"
				"\tret\n"
				"\n";
	}
	else
	{
		pc.error("unknown extern");
	}

	functions.insert({std::move(funcName), {std::move(argTypes), ""}});
}

Parser::Parser(const std::string& prog, std::stringstream& op)
	: pc(prog.c_str())
{
	op <<	"\tglobal main\n"
			"\textern ExitProcess\n"
			"\textern putchar\n"
			"\n"
			"section .text\n";

	while (true)
	{
		if (*pc == '\0') break;
		if (pc.tryParseWord("func"))
		{
			generateFunction(op);
		}
		else if (pc.tryParseWord("extern"))
		{
			generateExtern(op);
		}
		else
		{
			pc.error("unexpected symbol");
		}
	}
}