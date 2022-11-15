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

    virtual void on_message_callback(std::shared_ptr<message::Message> message) = 0;

    void run();
    
    virtual void work() = 0;
    
    MPI_Comm com;

    virtual void receive(Message &msg) {return;}
    void receive(ReplCrash &msg) override;
    void receive(ReplSpeed &msg) override;
    void receive(ReplStart &msg) override;

private:
        bool crashed;
        bool started;
        repl::ReplSpeed speed;
        std::string private_folder_location;

        std::deque<std::function<void()>> action_queue;
};
