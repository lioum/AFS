#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"

std::vector<std::string> split_words(std::string const &str, const char delim);
std::stringstream chars_to_stream(char* buffer, int size);
std::string readFileIntoString(const std::string& path);

template <typename Enumeration>
auto as_integer(Enumeration const value)
-> typename std::underlying_type<Enumeration>::type
{
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}