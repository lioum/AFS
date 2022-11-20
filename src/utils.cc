#include "utils.hh"

/*
** split_words Function
**
** @std::string const &str : string to be splited
** @const char delim : delimiter
**
** A simple split following a given delimiter
*/
std::vector<std::string> split_words(std::string const &str, const char delim)
{
    std::vector<std::string> inline_words;
    std::stringstream ss(str);
    std::string word;

    while (std::getline(ss, word, delim))
    {
        inline_words.push_back(word);
    }
    return inline_words;
}

/*
** chars_to_stream Function
**
** @char* buffer : data to be converted
** @int size : size of the buffer
**
** Convert a given buffer into a stringstream
*/
std::stringstream chars_to_stream(char *buffer, int size)
{
    std::stringstream ss("");
    for (int i = 0; i < size; i++)
    {
        if (buffer[i] == '\0')
        {
            return ss;
        }
        ss << buffer[i];
    }
    return ss;
}

/*
** readFileIntoString Function
**
** @std::string &path : file to be readed
**
** Read a file and convert it to string
*/
std::string readFileIntoString(const std::string &path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open())
    {
        std::cout << "Could not open the file - '" << path << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
    return std::string((std::istreambuf_iterator<char>(input_file)),
                       std::istreambuf_iterator<char>());
}

std::string serialize_command(std::shared_ptr<Command> cmd)
{
    if (cmd == nullptr)
        return "nullptr";
    return cmd->to_json().dump();
}

// Debug func to show entries
void show_entries(std::vector<LogEntry> entries)
{
    std::cerr << "{";
    for (size_t i = 0; i < entries.size(); i++)
    {
        if (i + 1 < entries.size())
            std::cerr << "{term: " << entries[i].term
                      << ", command: " << serialize_command(entries[i].command)
                      << "}, ";
        else
            std::cerr << "{term: " << entries[i].term
                      << ", command: " << serialize_command(entries[i].command)
                      << "}";
    }
    std::cerr << "}";
}