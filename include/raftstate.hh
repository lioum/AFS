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

struct RaftState
{
    // Raft timeout is between 150 and 300 ms in the reference paper
    const microseconds INITIAL_TIMEOUT = 150ms;

    explicit RaftState(int nb_servers, int uid);

    Role role = Role::FOLLOWER;
    size_t term = 0;
    int leader_uid = -1;

    chrono_time last_checked;
    microseconds timeout = INITIAL_TIMEOUT;
    int nb_servers;
    int uid;
};