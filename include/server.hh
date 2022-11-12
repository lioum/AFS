#pragma once

#include "raftstate.hh"

class Server {
    
public:
    Server(MPI_Comm com);

    std::shared_ptr<message::Message> listen();

    virtual void on_message_callback(std::shared_ptr<message::Message> message) = 0;

    void work();

    void run();

protected:
RaftState state;

};
