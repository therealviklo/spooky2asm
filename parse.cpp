#include "parse.h"

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

struct Splitter
{
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
			pc.skipNameOrNumber();
			if (pc.tryParse("(")) pc.skipParen();
			if (pc.tryParse(":")) pc.skipNameOrNumber();
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
		if (precRet.prec == 0) return true;
		if (precRet.prec >= currPrecedence)
		{
			first.setEnd(opStart);
			second = pc;
			op = precRet.op;
			currPrecedence = precRet.prec;
		}
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
};

std::string Parser::evaluateExpression(ParseCursor& pc, std::stringstream& op, FunctionData& fd)
{
	Splitter splitter(pc);

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
			const std::string name = splitter.first.readIdentifier();
			if (scope.has(name))
			{
				// if (!splitter.first.atEnd()) splitter.first.error("invalid syntax, expected '='");
			}
			else
			{
				if (functions.count(name)) splitter.first.error("variable name is already a function");
				if (!splitter.first.tryParse(":")) splitter.first.error("invalid syntax, expected ':'");
				const std::string typeName = splitter.first.readIdentifier();
				scope.add(name, {typeName, fd.localStack.getQword()});
				// if (!splitter.first.atEnd()) splitter.first.error("unexpected character");
			}
			
			if (evaluateExpression(splitter.second, op, fd).empty())
				splitter.second.error("expression does not return value");

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
					"\tcqo\n"
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
					"\tcqo\n"
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
			else if (splitter.first.tryParse("("))
			{
				return evaluateExpression(splitter.first, op, fd);
			}
			else if (splitter.first.tryParse("true"))
			{
				op << "\tmov rax, 1\n";
				return "Int";
			}
			else if (splitter.first.tryParse("false"))
			{
				op << "\txor rax, rax\n";
				return "Int";
			}
			else
			{
				const std::string id = splitter.first.readIdentifier();

				if (scope.has(id))
				{
					const std::string loc = scope.get(id).location;
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
			if (evaluateExpression(pc, op, fd).empty())
				pc.error("expression does not return value");
			if (!pc.tryParse(";")) pc.error("expected ';'");
		}
		op << "\tjmp .ret\n";
	}
	else if (pc.tryParseWord("if"))
	{
		const auto ifNum = labelManager.getIfNum();
		
		{
			LocalScope localScope(scope);

			if (!pc.tryParse("(")) pc.error("expected '('");
			if (evaluateExpression(pc, op, fd).empty())
				pc.error("expression does not return value");
			if (!pc.tryParse(")")) pc.error("expected ')'");

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
	// else if (pc.tryParseWord("while"))
	// {
	// 	const auto loopNum = labelManager.getLoopNum();

	// 	{
	// 		LocalScope localScope(scope);

	// 		op << ".wls" << loopNum << ":\n";

	// 		if (!pc.tryParse("(")) pc.error("expected '('");
	// 		evaluateExpression(pc, op, fd);
	// 		if (!pc.tryParse(")")) pc.error("expected ')'");
	// 		op <<	"\tcmp rax, 0\n"
	// 				"\tje .wle" << loopNum << "\n";

	// 		if (pc.tryParse("{"))
	// 		{
	// 			generateBlock(op, fd);
	// 		}
	// 		else
	// 		{
	// 			generateStatement(op, fd);
	// 		}

	// 		op <<	"\tjmp .wls" << loopNum << "\n"
	// 				".wle" << loopNum << ":\n";
	// 	}
	// }
	else if (pc.tryParseWord("for"))
	{
		const auto loopNum = labelManager.getLoopNum();

		{
			LocalScope localScope(scope);

			if (!pc.tryParse("(")) pc.error("expected '('");
			
			if (!pc.tryParse(";"))
			{
				evaluateExpression(pc, op, fd);
				if (!pc.tryParse(";")) pc.error("expected ';'");
			}

			op << ".fls" << loopNum << ":\n";

			if (!pc.tryParse(";"))
			{
				evaluateExpression(pc, op, fd);
				op <<	"\tcmp rax, 0\n"
						"\tje .fle" << loopNum << "\n";
				if (!pc.tryParse(";")) pc.error("expected ';'");
			}

			std::stringstream endExpr;

			if (!pc.tryParse(")"))
			{
				evaluateExpression(pc, endExpr, fd);
				if (!pc.tryParse(")")) pc.error("expected ')'");
			}

			if (pc.tryParse("{"))
			{
				generateBlock(op, fd);
			}
			else
			{
				generateStatement(op, fd);
			}

			op << endExpr.str();
			op <<	"\tjmp .fls" << loopNum << "\n"
					".fle" << loopNum << ":\n";
		}
	}
	else
	{
		if (!pc.tryParse(";"))
		{
			evaluateExpression(pc, op, fd);
			if (!pc.tryParse(";")) pc.error("expected ';'");
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
	if (functions.count(funcName) > 1) pc.error("function redefinition");
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
		if (fd.retType == "Void") fd.retType = "";
	}

	if (funcName == "main")
	{
		if (argTypes.size()) pc.error("invalid argument list for main");
		if (!fd.retType.empty()) pc.error("invalid return type for main");
	}

	// functions.insert({funcName, {std::move(argTypes), fd.retType}});

	if (!pc.tryParse("{")) pc.error("expected '{'");

	std::stringstream body;
	generateBlock(body, fd);

	op << getFuncLabel(funcName) << ":\n";
	
	if (funcName == "main")
	{
		op <<	"\tsub rsp, 40\n"
				"\txor rcx, rcx\n"
				"\tcall time\n"
				"\tmov rcx, rax\n"
				"\tcall srand\n"
				"\tadd rsp, 40\n";
	}

	op << fd.localStack.getStackInit().str();

	if (funcName == "main")
	{
		op << "\tcall spookyInitGlobals\n";
	}

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
	if (functions.count(funcName) > 1) pc.error("function redefinition");
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
		if (retType == "Void") retType = "";
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
	else if (funcName == "random")
	{
		if (!checkTypes({})) pc.error("invalid extern arguments");
		if (retType != "Int") pc.error("invalid extern return type");
		op <<	"_random:\n" <<
				R"(	push rbp
	mov rbp, rsp
	xor rdx, rdx
	mov rax, rsp
	mov rcx, 16
	div rcx
	sub rsp, rdx
	xor rax, rax
	sub rsp, 40
	call rand
	add rsp, 40
	and rax, 0xFF
	sub rsp, 32
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	push rax
	call rand
	and rax, 0xFF
	pop rcx
	shl rcx, 8
	add rax, rcx
	add rsp, 32
	mov rsp, rbp
	pop rbp
	ret
)";
	}
	else
	{
		pc.error("unknown extern");
	}

	// functions.insert({std::move(funcName), {std::move(argTypes), retType}});
}

void Parser::addGlobal()
{
	std::string varName = pc.readIdentifier();

	if (functions.count(varName)) pc.error("function redefinition");
	if (scope.has(varName)) pc.error("redefinition of variable");

	if (!pc.tryParse(":")) pc.error("expected ':'");
	
	std::string typeName = pc.readIdentifier();
	
	if (!pc.tryParse("=")) pc.error("expected '='");
	
	evaluateExpression(pc, globalsFunc, globalsFuncData);
	globalsFunc << "\tmov qword [qword g_" << varName << "], rax\n";
	bssSection << "\tg_" << varName << ": resq 1\n";
	scope.add(varName, {typeName, "qword [qword g_" + varName + "]"});
}

Parser::Parser(const std::string& prog, std::stringstream& op)
	: pc(prog.c_str())
{
	// ppc läser in funktionerna i förväg
	ParseCursor ppc = pc;
	while (*ppc != '\0')
	{
		auto readFunctionInfo = [&]() {
			std::string funcName = ppc.readIdentifier();

			if (!ppc.tryParse("(")) ppc.error("expected '('");
			std::vector<std::string> argTypes;
			if (!ppc.tryParse(")"))
			{
				do
				{
					ppc.readIdentifier();
					if (!ppc.tryParse(":")) ppc.error("expected ':'");
					argTypes.push_back(ppc.readIdentifier());
				} while (ppc.tryParse(","));
				if (!ppc.tryParse(")")) ppc.error("expected ')'");
			}

			std::string retType;
			if (ppc.tryParse("->"))
			{
				retType = ppc.readIdentifier();
				if (retType == "Void") retType = "";
			}

			functions.insert({std::move(funcName), {std::move(argTypes), std::move(retType)}});
		};

		if (ppc.tryParseWord("func"))
		{
			readFunctionInfo();
			if (!ppc.tryParse("{")) ppc.error("expected '{");
			ppc.skipBlock();
		}
		else if (ppc.tryParseWord("extern"))
		{
			readFunctionInfo();
		}
		else
		{
			ppc.readIdentifier();
			if (!ppc.tryParse(":")) ppc.error("expected ':'");
			ppc.readIdentifier();
			if (!ppc.tryParse("=")) ppc.error("expected '='");
			Splitter splitter(ppc);
		}
	}

	op <<	"\tglobal main\n"
			"\textern ExitProcess\n"
			"\textern putchar\n"
			"\textern rand\n"
			"\textern srand\n"
			"\textern time\n"
			"\n"
			"section .text\n";

	bssSection << "section .bss\n";

	LocalScope globalVarScope(scope);

	while (*pc != '\0')
	{
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
			addGlobal();
		}
	}

	op << "spookyInitGlobals:\n";
	op << globalsFuncData.localStack.getStackInit().str();
	op << globalsFunc.str();
	op <<	"\tmov rsp, rbp\n"
			"\tpop rbp\n"
			"\tret\n";
	op << bssSection.str();

	if (!functions.count("main")) pc.error("no main function");
}