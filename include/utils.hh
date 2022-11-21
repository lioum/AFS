#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <random>

#include "nlohmann/json.hpp"
#include "message.hh"
#include "processus.hh"

/*
** random_election_timeout Function
**
** Returns a timeout between 150ms and 300ms according to raft paper
*/
nanoseconds random_election_timeout();

/*
** split_words Function
**
** @std::string const &str : string to be splited
** @const char delim : delimiter
**
** A simple split following a given delimiter
*/
std::vector<std::string> split_words(std::string const &str, const char delim);


/*
** chars_to_stream Function
**
** @char* buffer : data to be converted
** @int size : size of the buffer
**
** Convert a given buffer into a stringstream
*/
std::stringstream chars_to_stream(char* buffer, int size);


/*
** readFileIntoString Function
**
** @std::string &path : file to be readed
**
** Read a file and convert it to string
*/
std::string readFileIntoString(const std::string& path);


/*
** show_entries Function
**
** @std::vector<LogEntry> &path : vector of entries to be shown
**
** Debug func to show entries
*/
void show_entries(std::vector<LogEntry> entries);

template <typename Enumeration>
auto as_integer(Enumeration const value)
-> typename std::underlying_type<Enumeration>::type
{
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}