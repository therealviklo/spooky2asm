﻿#pragma once
#include <string>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <unordered_map>

bool validNameChar(char c) noexcept;
std::string getFuncLabel(const std::string& func);

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

class ParseCursor
{
private:
	const char* cur;
	size_t x;
	size_t y;
public:
	ParseCursor(const char* str);

	void move();
	void move(size_t num);
	void skipWhitespace() noexcept;

	bool tryParse(const char* cmpStr) noexcept;
	bool tryParseWord(const char* cmpStr) noexcept;
	std::string readIdentifier();

	inline void error(const char* msg);

	constexpr operator const char*() const noexcept {return cur;}
};

class LocalStack
{
private:
	size_t stackSize;
public:
	LocalStack() : stackSize(0) {}

	std::string getQword();

	std::stringstream getStackInit();
};

struct Variable
{
	std::string type;
	std::string location;
};

class LocalScope;
class Scope
{
private:
	std::vector<std::unordered_map<std::string, Variable>> scopes;
public:
	bool has(std::string var) const;
	Variable& get(std::string var);

	void add(std::string varName, Variable var);

	void addScope();
	void removeScope() noexcept;
};
class LocalScope
{
private:
	Scope& scope;
public:
	LocalScope(Scope& scope);
	~LocalScope();

	LocalScope(const LocalScope&) = delete;
	LocalScope& operator=(const LocalScope&) = delete;
};

void evaluateExpression(ParseCursor& pc, std::stringstream& op, LocalStack& localStack, Scope& scope);
void generateFunction(ParseCursor& pc, std::stringstream& op);

std::stringstream compile(const std::string& prog);