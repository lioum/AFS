#include <chrono>
#include <cstddef>

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
    int uid;
    MPI_Comm comm;

public:
    RaftState(MPI_Comm comm);

    inline bool is_leader()
    {
        return leader_uid == uid;
    }

    void update();
};
