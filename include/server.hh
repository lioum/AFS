#pragma once

#include "raftstate.hh"

class Server {
    
public:
    Server(MPI_Comm com);
    RaftState state;

    void work();
    
private:
    int rank;
    int size;
};
