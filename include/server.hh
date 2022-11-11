#pragma once

#include "raftstate.hh"

class Server {
    
public:
    RaftState state;

    void work();
};
