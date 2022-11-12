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
      std::string input;
      bool running = true;
      while (running) {
        std::cout << "REPL(" << state.get_rank() << ")$ ";
        std::cin >> input;
        auto bite = process_message(input);
        input.clear();
        if (bite.has_value()) {
          auto serialization = bite->serialize();
          int err =
              MPI_Send(serialization.data(), serialization.length(), MPI_CHAR,
                       bite->get_target_rank(), 0, state.get_comm());
          if (err != 0) {
            char *error_string =
                (char *)malloc((sizeof(char)) * MPI_MAX_ERROR_STRING);
            int len;
            MPI_Error_string(err, error_string, &len);
            std::cout << "Send: " << error_string << std::endl;
          }
          //running = false;
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
      if (input == "CRASH") // DEAFEN
      {
        std::cout << "REPL: Crashing which one ? ";
        std::cin >> res;
      try {
        target_rank = std::stoi(res);
      } catch (std::exception e) {
        std::cout << "REPL: Invalid rank" << std::endl;
        return std::nullopt;
      }
        return std::make_optional<REPL_message>(ReplType::CRASH, target_rank);
      } else if (input == "SPEED") {
        std::cout << "REPL: changing speed of which one ? ";
        std::cin >> res;
      try {
        target_rank = std::stoi(res);
      } catch (std::exception e) {
        std::cout << "REPL: Invalid rank" << std::endl;
        return std::nullopt;
      }
        std::string speed;
        std::cout << "REPL: What speed (low, medium, high) ? ";
        std::cin >> speed;
        if (speed != "low" && speed != "medium" && speed != "high") {
          std::cout << "REPL: Invalid speed" << std::endl;
          return std::nullopt;
        }
        return std::make_optional<REPL_message>(target_rank, speed);
      } else if (input == "START") {
        std::cout << "REPL: Starting Which one ? ";
        std::cin >> res;
      try {
        target_rank = std::stoi(res);
      } catch (std::exception e) {
        std::cout << "REPL: Invalid rank" << std::endl;
        return std::nullopt;
      }
        return std::make_optional<REPL_message>(ReplType::START, target_rank);
      }
        return std::nullopt;
    }
}