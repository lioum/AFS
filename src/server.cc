#include "server.hh"
#include <mpi.h>
#include "raftstate.hh"
#include "repl.hh"

Server::Server(MPI_Comm com, int nb_servers)
    : state(com, nb_servers)
{
}


std::shared_ptr<message::Message> Server::listen()
{
    int flag;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, state.get_comm(), &flag, &status);
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

void Server::send(int target_rank, std::shared_ptr<message::Message> message)
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

void Server::run()
{
    while (true)
    {
        std::shared_ptr<message::Message> message = listen();
        if (message.get() != nullptr)
        {
          on_message_callback(message);
        }
        work();
    }
}