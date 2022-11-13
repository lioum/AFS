#pragma once

#include "raftstate.hh"

class Server {
    
public:
    Server(MPI_Comm com, int nb_servers);

    std::shared_ptr<message::Message> listen();

    void send(int target_rank, std::shared_ptr<message::Message> message);

    virtual void on_message_callback(std::shared_ptr<message::Message> message) = 0;

    void run();
    
    virtual void work() = 0;

protected:
RaftState state;

};
