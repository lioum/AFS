#include "repl.hh"

#include <exception>
#include <memory>

#include "repl_message.hh"

namespace repl
{

    void REPL::run()
    {
        while (true)
        {
            std::cout << "REPL(" << state.get_rank() << "): " << std::endl;
            std::cout << "REPL$ ";
            std::string input;
            std::cin >> input;
            auto bite = process_message(input);
            serialize_and_send(bite);
        }
    }

    std::optional<REPL_message> REPL::process_message(std::string input)
    {
        std::string res;
        int target_rank;
        if (input == "CRASH") // DEAFEN
        {
            std::cout << "REPL: Which one ? ";
            std::cin >> res;
            try
            {
                target_rank = std::stoi(res);
            }
            catch (std::exception e)
            {
                std::cout << "REPL: Invalid rank" << std::endl;
                return std::nullopt;
            }
            return std::make_optional<REPL_message>(ReplType::CRASH, target_rank);
        }
        else if (input == "SPEED")
        {}
        else if (input == "START")
        {}
        return std::nullopt;
    }
}