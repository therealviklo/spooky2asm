#pragma once
#include <string>

bool validNameChar(char c) noexcept;
std::string getFuncLabel(const std::string& func);
bool peekcmp(const char* str, const char* cmp) noexcept;