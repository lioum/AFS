#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>


std::vector<std::string> split_words(std::string const &str, const char delim);
std::stringstream chars_to_stream(char* buffer, int size);
std::string readFileIntoString(const std::string& path);