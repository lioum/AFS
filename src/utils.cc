#include "utils.hh"

nanoseconds random_election_timeout()
{
    std::random_device rd;
    std::uniform_int_distribution<long int> dist(150 * 10e5, 300 * 10e5);

    return nanoseconds(dist(rd));
}

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

void show_entries(std::vector<LogEntry> entries)
{
    std::cerr << "{";
    for (size_t i = 0; i < entries.size(); i++)
    {
        std::cerr << "{term: " << entries[i].term << ", command: ";
        if (entries[i].command == nullptr)
            std::cerr << "null";
        else
            std::cerr << entries[i].command->to_json()["type"];

        if (i + 1 < entries.size())
            std::cerr << "}, ";
        else
            std::cerr << "}";
    }
    std::cerr << "}";
}