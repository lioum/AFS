#include "repl.hh"

#include <cstdlib>
#include <exception>
#include <memory>
#include <mpi.h>
#include <optional>

#include "repl_message.hh"

namespace repl
{
    
    REPL::REPL(MPI_Comm com)
    : Server(com)
    {
    }

    void REPL::run()
    {
        while(true)
        {
            std::cout << "REPL(" << state.get_rank() << "): " << std::endl;
            std::cout << "REPL$ ";
            std::string input;
            std::cin >> input;
            auto bite = process_message(input);
            if (bite.has_value())
            {
                auto serialization = bite->serialize();
                int err = MPI_Send(serialization.data(), serialization.length(), MPI_CHAR, bite->get_target_rank(), 0, state.get_comm());
                char* error_string = (char*)malloc((sizeof(char)) * MPI_MAX_ERROR_STRING);
                int len;
                MPI_Error_string(err, error_string, &len);
                std::cout << error_string << std::endl;
            }
            else {
                std::cout << "MOGUS" << std::endl;
            }
        }
    }

    void REPL::on_message_callback(std::shared_ptr<message::Message> message)
    {
        std::cout << "REPL: Received message" << std::endl;
    }

    std::optional<REPL_message> REPL::process_message(std::string input)
    {
        std::string res;
        int target_rank;
        std::cout << "REPL: Which one ? ";
        std::cin >> res;
        try {
          target_rank = std::stoi(res);
        } catch (std::exception e) {
          std::cout << "REPL: Invalid rank" << std::endl;
          return std::nullopt;
        }
        if (input == "CRASH") // DEAFEN
        {
            return std::make_optional<REPL_message>(ReplType::CRASH, target_rank);
        }
        else if (input == "SPEED")
        {
            std::string speed;
            std::cout << "REPL: What speed (low, medium, high) ? ";
            std::cin >> speed;            
            if (speed != "low" && speed != "medium" && speed != "high")
            {
                std::cout << "REPL: Invalid speed" << std::endl;
                return std::nullopt;
            }
            return std::make_optional<REPL_message>(target_rank, speed);
        }
        else if (input == "START")
        {
            return std::make_optional<REPL_message>(ReplType::START, target_rank);
        }
        return std::nullopt;
    }
}