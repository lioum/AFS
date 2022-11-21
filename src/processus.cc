#include "processus.hh"

#include <mpi.h>

#include "repl.hh"
#include "utils.hh"

Processus::Processus(int nb_servers)
    : nb_servers(nb_servers)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &uid);
}

std::shared_ptr<Message> Processus::listen()
{
    int flag;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    int source = status.MPI_SOURCE;
    auto tag = status.MPI_TAG;

    if (!flag)
        return nullptr;

    // Get the size of the top element of the source
    int count = 0;
    MPI_Get_count(&status, MPI_CHAR, &count);

    // Receiving the listened message from the json in a buffer
    auto buffer = std::vector<char>(count);
    int err =
        MPI_Recv(buffer.data(), count, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
    if (err != 0)
    {
        char *error_string =
            (char *)malloc((sizeof(char)) * MPI_MAX_ERROR_STRING);
        int len;
        MPI_Error_string(err, error_string, &len);
        std::cout << "Receiving: " << error_string << std::endl;
    }
    std::shared_ptr<Message> message = Message::deserialize(json::parse(buffer));
    return message;
}

void Processus::send(const Message &msg)
{
    auto serialization = msg.serialize();
    int err = MPI_Send(serialization.data(), serialization.length(), MPI_CHAR,
                       msg.target_rank, 0, MPI_COMM_WORLD);
    
    if (err != 0)
    {
        char *error_string =
            (char *)malloc((sizeof(char)) * MPI_MAX_ERROR_STRING);
        int len;
        MPI_Error_string(err, error_string, &len);
        std::cout << "Send: " << error_string << std::endl;
    }
}

void Processus::broadcast_to_servers(Message &message)
{
    // LOG ACTION TO LOGFILE (useful in order to recover changes for crashed
    // server coming back online)
    for (int i = 1; i <= nb_servers; i++)
    {
        if (i != uid)
        {
            message.target_rank = i;
            send(message);
        }
    }
}

void Processus::run()
{
    while (true)
    {
        std::shared_ptr<Message> message = listen();
        if (message.get() != nullptr)
        {
            message->accept(*this);
        }
        work();
    }
}

InternProcessus::InternProcessus(int nb_servers,
                                 const std::filesystem::path &root_folder_path)
    : Processus(nb_servers)
    , crashed(false)
    , started(false)
    , speed(Speed::FAST)
{
    int speed_val = as_integer(speed);
    sleeping_time = speed_val * speed_val * std::chrono::milliseconds(1000);

    working_folder_path = root_folder_path / std::to_string(uid);

    if (!std::filesystem::exists(working_folder_path))
        std::filesystem::create_directory(working_folder_path);
}

void InternProcessus::receive(ReplCrash &msg)
{
    std::cout << "Processus(" << uid << ") is crashing." << std::endl;
    crashed = true;
    send(HandshakeSuccess(msg.sender_rank, uid));
}

void InternProcessus::receive(ReplSpeed &msg)
{
    std::cout << "Processus(" << uid << ") is changing speed from "
              << as_integer(speed) << " to " << as_integer(msg.speed)
              << std::endl;
    speed = msg.speed;
    int speed_val = as_integer(speed);
    sleeping_time = speed_val * speed_val * std::chrono::milliseconds(1000);
    send(HandshakeSuccess(msg.sender_rank, uid));
}

void InternProcessus::receive(ReplStart &msg)
{
    std::cout << "Processus(" << uid << ") is starting" << std::endl;
    started = true;
    send(HandshakeSuccess(msg.sender_rank, uid));
}

void InternProcessus::receive(ReplRecovery &msg)
{
    std::cout << "Processus(" << uid << ") is coming back from the dead" << std::endl;
    crashed = false;
    send(HandshakeSuccess(msg.sender_rank, uid));
}

void InternProcessus::receive(RpcMessage &msg)
{
    //std::cout << "Processus(" << uid << ") is starting" << std::endl;
    send(HandshakeSuccess(msg.sender_rank, uid));
}
