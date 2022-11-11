#include "server.hh"
#include <mpi.h>
#include "raftstate.hh"

Server::Server(MPI_Comm com)
: state(RaftState(com))
{}