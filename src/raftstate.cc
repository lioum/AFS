#include "raftstate.hh"

#include <ctime>
#include <mpi.h>

RaftState::RaftState(int nb_servers)
    : role(Role::FOLLOWER)
    , term(0)
    , leader_uid(-1)
    , last_checked(std::chrono::system_clock::now())
    , timeout(RaftState::INITIAL_TIMEOUT)
    , nb_servers(nb_servers)
{}