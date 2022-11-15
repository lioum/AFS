#include "processus.hh"
#include <mpi.h>
#include "raftstate.hh"
#include "repl.hh"

Processus::Processus(MPI_Comm com)
    : com(com)
    , crashed(false)
    , started(false)
    , speed(repl::ReplSpeed::FAST)
{}


std::shared_ptr<message::Message> Processus::listen()
{
    int flag;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    int source = status.MPI_SOURCE;
    auto tag = status.MPI_TAG;

    if (!flag)
        return nullptr;

    int count = 0;
    MPI_Get_count(&status, MPI_CHAR, &count);

    auto buffer = std::vector<char>(count);
    int err = MPI_Recv(buffer.data(), count, MPI_CHAR, source, tag, state.get_comm(), &status);
    if (err != 0)
    {
      char *error_string =
          (char *)malloc((sizeof(char)) * MPI_MAX_ERROR_STRING);
      int len;
      MPI_Error_string(err, error_string, &len);
      std::cout << "Receiving: " << error_string << std::endl;
    }
    std::string s(buffer.begin(), buffer.end());
    std::shared_ptr<message::Message> message = message::Message::deserialize(s);
    return message;
}

void Processus::send(int target_rank, std::shared_ptr<message::Message> message)
{
    auto serialization = message->serialize();
    int err =
        MPI_Send(serialization.data(), serialization.length(), MPI_CHAR,
                 target_rank, 0, state.get_comm());
    if (err != 0)
    {
      char *error_string =
          (char *)malloc((sizeof(char)) * MPI_MAX_ERROR_STRING);
      int len;
      MPI_Error_string(err, error_string, &len);
      std::cout << "Send: " << error_string << std::endl;
    }
}

void Processus::handshake_success(int target_rank)
{
    auto response = std::make_shared<message::HandshakeSuccess>(target_rank,
     state.get_rank());
    send(target_rank, response);
}

void Processus::handshake_success(int target_rank, json data)
{
    auto response = std::make_shared<message::HandshakeSuccess>(target_rank,
     state.get_rank(), data);
    send(target_rank, response);
}

void Processus::run()
{
    while (true)
    {
        std::shared_ptr<message::Message> message = listen();
        if (message.get() != nullptr)
        {
            message->accept(this);
        }
        work();
    }
}

void Processus::receive(ReplCrash &msg)
{
    std::cout << "Processus(" << state.get_rank()
              << ") is crashing. Bravo Six, going dark" << std::endl;

    crashed = true;
    handshake_success(msg.sender_rank);
}

void Processus::receive(ReplSpeed &msg)
{
    std::cout << "Processus(" << state.get_rank()
              << ") is changing speed from " << speed << " to "
              << msg.speed << std::endl;

    speed = msg.speed;
    handshake_success(msg.sender_rank);
}

void Processus::receive(ReplStart &msg)
{
    std::cout << "Processus(" << state.get_rank() 
              << ") is starting" << std::endl;

    started = true;
    handshake_success(msg.sender_rank);
}