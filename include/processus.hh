#pragma once

#include <chrono>
#include <cstddef>
#include <deque>
#include <mpi.h>
#include <optional>

class Message;

class Processus
{
public:
    Processus(MPI_Comm com);

    std::shared_ptr<Message> listen();

    void send(int target_rank, std::shared_ptr<Message> message);

    void handshake_success(int target_rank);
    void handshake_success(int target_rank, json data);

    void run(); 
    virtual void work() = 0;

    MPI_Comm com;

    virtual void receive(Message &msg)
    {}
    virtual void receive(ReplCrash &msg){};
    virtual void receive(ReplSpeed &msg){};
    virtual void receive(ReplStart &msg){};

    virtual void execute(QueueMessage &msg) {return;}

    MPI_Comm com;

private:
    bool crashed;
    bool started;
    ReplSpeed speed;
    std::string private_folder_location;

    std::deque<QueueMessage> action_queue;
};