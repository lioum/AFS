#include "raftstate.hh"

#include <ctime>
#include <mpi.h>

RaftState::RaftState(MPI_Comm comm, int nb_servers)
    : role(Role::FOLLOWER)
    , clock(0)
    , leader_uid(-1)
    , last_checked(std::chrono::system_clock::now())
    , timeout(RaftState::INITIAL_TIMEOUT)
    , comm(comm)
    , nb_servers(nb_servers)

{
    MPI_Comm_rank(comm, &uid);
    MPI_Comm_size(comm, &nb_states);
}

void RaftState::update()
{
    if (role == Role::FOLLOWER)
    {
        auto time_delta = std::chrono::system_clock::now() - last_checked;
        timeout -= std::chrono::duration_cast<microseconds>(time_delta);

        last_checked = std::chrono::system_clock::now();

        if (timeout <= 0us)
        {
            MPI_Request request;
            MPI_Ibcast(&clock, 1, MPI_UNSIGNED_LONG, MPI_ROOT, comm, &request);

            timeout = RaftState::INITIAL_TIMEOUT;
            role = Role::CANDIDATE;
        }
    }
}
