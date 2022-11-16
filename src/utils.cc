#include "utils.hh"

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

std::string readFileIntoString(const std::string &path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open())
    {
        std::cerr << "Could not open the file - '" << path << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
    return std::string((std::istreambuf_iterator<char>(input_file)),
                       std::istreambuf_iterator<char>());
}