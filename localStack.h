#pragma once
#include <string>
#include <sstream>

class LocalStack
{
private:
	size_t stackSize;
public:
	LocalStack() : stackSize(0) {}

	std::string getQword();

	std::stringstream getStackInit();
};