#pragma once
#include <string>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <unordered_map>
#include "parseUtils.h"
#include "parseExceptions.h"
#include "parseCursor.h"
#include "localStack.h"
#include "func.h"
#include "scope.h"

enum ExprOp
{
	EO_NOOP,
	EO_EQ,  	// ==
	EO_ASSIGN,	// =
	EO_GE,  	// >=
	EO_GT,  	// >
	EO_LE,  	// <=
	EO_LT,  	// <
	EO_ADD, 	// +
	EO_SUB, 	// -
	EO_MOD, 	// %
	EO_MUL, 	// *
	EO_DIV  	// /
};

class Parser
{
private:
	ParseCursor pc;
	Functions functions;
	Scope scope;
	class LabelManager {
		private:
			size_t loopNum;
			size_t ifNum;
		public:
			LabelManager() : loopNum(0), ifNum(0) {};

			size_t getLoopNum() noexcept { return loopNum++; }
			size_t getIfNum() noexcept { return ifNum++; }
	} labelManager;
	std::stringstream globalsFunc;
	FunctionData globalsFuncData;
	std::stringstream bssSection;

	// Return: typen
	std::string evaluateExpression(ParseCursor& pc, std::stringstream& op, FunctionData& fd);
	void generateStatement(std::stringstream& op, FunctionData& fd);
	void generateBlock(std::stringstream& op, FunctionData& fd);

	void generateFunction(std::stringstream& op);
	void generateExtern(std::stringstream& op);
	void addGlobal();
public:
	Parser(const std::string& prog, std::stringstream& op);
};

inline std::stringstream compile(const std::string& prog)
{
	std::stringstream op;

	Parser p(prog, op);

	return op;
}