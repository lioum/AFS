#pragma once

#include "client_message.hh"

#include <chrono>
#include <cstddef>
#include <optional>

class Processus {
    
public:
    Processus(MPI_Comm com);

    std::shared_ptr<message::Message> listen();

    void send(int target_rank, std::shared_ptr<message::Message> message);
    void handshake_success(int target_rank);
    void handshake_success(int target_rank, json data);

    void run(); 
    virtual void work() = 0;
    
    virtual void receive(Message &msg) {return;}
    void receive(ReplCrash &msg) override;
    void receive(ReplSpeed &msg) override;
    void receive(ReplStart &msg) override;

    virtual void execute(QueueMessage &msg) {return;}

    MPI_Comm com;

private:
    bool crashed;
    bool started;
    repl::ReplSpeed speed;
    std::string private_folder_location;

    std::deque<QueueMessage> action_queue;
};
