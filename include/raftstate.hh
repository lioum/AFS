#pragma once

#include <chrono>
#include <cstddef>
#include <optional>

#include <mpi.h>
#include "message.hh"

enum class Role
{
    CANDIDATE,
    FOLLOWER,
    LEADER
};

using chrono_time = std::chrono::time_point<std::chrono::system_clock>;
using std::chrono::microseconds;
using namespace std::chrono_literals;

class RaftState
{

    const microseconds INITIAL_TIMEOUT = 10000us;

private:
    Role role;
    size_t clock;
    int leader_uid;

    chrono_time last_checked;
    microseconds timeout;
    int nb_states;
    MPI_Comm comm;
    
    int nb_servers;

protected:
    int uid;

public:
    RaftState(MPI_Comm comm, int nb_servers);

    inline bool is_leader()
    {
        return leader_uid == uid;
    }
    
    inline int get_rank()
    {
        return uid;
    }

    inline Role get_status()
    {
        return role;
    }

    inline MPI_Comm get_comm()
    {
        return comm;
    }

    inline int get_nb_servers()
    {
        return nb_servers;
    }

    void update();
};
