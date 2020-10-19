#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "localStack.h"

struct Function
{
	std::vector<std::string> argTypes;
	std::string retType;
};
typedef std::unordered_map<std::string, Function> Functions;

struct FunctionData
{
	LocalStack localStack;
	std::string retType;
};