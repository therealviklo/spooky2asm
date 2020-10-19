#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "parseExceptions.h"

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