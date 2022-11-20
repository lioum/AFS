#include "repl.hh"

#include <cstdlib>
#include <exception>
#include <memory>
#include <mpi.h>
#include <optional>

REPL::REPL(MPI_Comm com, int nb_servers, int nb_clients)
    : Processus(com, nb_servers)
    , nb_clients(nb_clients)
{}

void REPL::work()
{
    std::string input;
    if (!waiting_for_handshake && !std::cin.eof())
    {
        std::cerr << "REPL{" << uid << "} (START / CRASH / LOAD / DELETE / RECOVERY)$ ";
        std::cin >> input;
        std::shared_ptr<Message> repl_command = process_message(input);

        if (repl_command->target_rank > 0 && 
            repl_command->target_rank <= (nb_clients + nb_servers) && 
            repl_command != nullptr)
        {
            send(*repl_command);
            waiting_for_handshake = true;
        }
    }
}

void REPL::receive(HandshakeFailure &msg)
{
    if (waiting_for_handshake)
    {
        std::cerr << "REPL " << uid << " received handshake failure from server "
              << msg.sender_rank <<std::endl;
        waiting_for_handshake = false;
    }
}

void REPL::receive(HandshakeSuccess &msg)
{
    if (waiting_for_handshake)
    {
        std::cerr << "REPL " << uid << " received handshake success from server "
              << msg.sender_rank <<std::endl;
        waiting_for_handshake = false;
    }
}

std::shared_ptr<Message> REPL::process_message(const std::string &input)
{
    std::string res;
    int target_rank;
    if (input == "CRASH") // DEAFEN
    {
        std::cerr << "REPL: Crashing which one ? ";
        std::cin >> res;
        try
        {
            target_rank = std::stoi(res);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "REPL: Invalid rank" << std::endl;
            return nullptr;
        }
        return std::make_shared<ReplCrash>(target_rank, uid);
    }
    else if (input == "SPEED")
    {
        std::cerr << "REPL: changing speed of which one ? ";
        std::cin >> res;
        try
        {
            target_rank = std::stoi(res);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "REPL: Invalid rank" << std::endl;
            return nullptr;
        }
        std::string speed_str;
        std::cerr << "REPL: What speed (fast, medium, low) ? ";
        std::cin >> speed_str;
        if (speed_str != "low" && speed_str != "medium" && speed_str != "fast")
        {
            std::cerr << "REPL: Invalid speed" << std::endl;
            return nullptr;
        }
        Speed speed = Speed::FAST;
        if (speed_str == "medium")
        {
            speed = Speed::MEDIUM;
        }
        else if (speed_str == "low")
        {
            speed = Speed::LOW;
        }
        return std::make_shared<ReplSpeed>(target_rank, uid, speed);
    }
    else if (input == "START")
    {
        std::cerr << "REPL: Starting Which one ? ";
        std::cin >> res;
        try
        {
            target_rank = std::stoi(res);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "REPL: Invalid rank" << std::endl;
            return nullptr;
        }
        return std::make_shared<ReplStart>(target_rank, uid);
    }
    else if (input == "RECOVERY")
    {
        std::cerr << "REPL: Recovery Which one ? ";
        std::cin >> res;
        try
        {
            target_rank = std::stoi(res);
        }
        catch (std::invalid_argument &e)
        {
            std::cerr << "REPL: Invalid rank" << std::endl;
            return nullptr;
        }
        return std::make_shared<ReplRecovery>(target_rank, uid);
    }
    return nullptr;
}