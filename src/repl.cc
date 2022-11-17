#include "repl.hh"

#include <cstdlib>
#include <exception>
#include <memory>
#include <mpi.h>
#include <optional>

REPL::REPL(MPI_Comm com, int nb_servers)
    : Processus(com, nb_servers)
{}

void REPL::work()
{
    std::string input;
    if (!waiting_for_handshake)
    {
        std::cout << "REPL(" << uid << ")$ ";
        std::cin >> input;
        std::shared_ptr<Message> bite = process_message(input);
        if (bite != nullptr)
        {
            send(*bite);
            waiting_for_handshake = true;
        }
    }
}

void REPL::receive(HandshakeFailure &msg)
{
    if (waiting_for_handshake)
    {
        std::cout << "REPL " << uid << " received handshake failure from server "
              << msg.sender_rank <<std::endl;
        waiting_for_handshake = false;
    }
}

void REPL::receive(HandshakeSuccess &msg)
{
    if (waiting_for_handshake)
    {
        std::cout << "REPL " << uid << " received handshake success from server "
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
        std::cout << "REPL: Crashing which one ? ";
        std::cin >> res;
        try
        {
            target_rank = std::stoi(res);
        }
        catch (std::invalid_argument &e)
        {
            std::cout << "REPL: Invalid rank" << std::endl;
            return nullptr;
        }
        return std::make_shared<ReplCrash>(target_rank, uid);
    }
    else if (input == "SPEED")
    {
        std::cout << "REPL: changing speed of which one ? ";
        std::cin >> res;
        try
        {
            target_rank = std::stoi(res);
        }
        catch (std::invalid_argument &e)
        {
            std::cout << "REPL: Invalid rank" << std::endl;
            return nullptr;
        }
        std::string speed_str;
        std::cout << "REPL: What speed (fast, medium, low) ? ";
        std::cin >> speed_str;
        if (speed_str != "low" && speed_str != "medium" && speed_str != "fast")
        {
            std::cout << "REPL: Invalid speed" << std::endl;
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
        std::cout << "REPL: Starting Which one ? ";
        std::cin >> res;
        try
        {
            target_rank = std::stoi(res);
        }
        catch (std::invalid_argument &e)
        {
            std::cout << "REPL: Invalid rank" << std::endl;
            return nullptr;
        }
        return std::make_shared<ReplStart>(target_rank, uid);
    }
    return nullptr;
}