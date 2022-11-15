#pragma once

#include <queue>

#include <mpi.h>
#include "repl_message.hh"
#include "rpc.hh"
#include "processus.hh"

using chrono_time = std::chrono::time_point<std::chrono::system_clock>;
using std::chrono::milliseconds;
using namespace std::chrono_literals;

namespace raft
{
    enum class Role
    {
        CANDIDATE,
        FOLLOWER,
        LEADER
    };
    
    class RaftServer : public Processus
    {
    public:
        RaftServer(MPI_Comm com, int nb_servers);

        void work() override;
        
        void receive(RpcRequestVote &msg) override;
        void receive(RpcAppendEntries &msg) override;
        void receive(RpcVoteResponse &msg) override;
        void receive(RpcAppendEntriesResponse &msg) override;
        void receive(HandshakeFailure &msg) override;
        void receive(HandshakeSuccess &msg) override;
        void receive(ClientLoad &msg) override;
        void receive(ClientList &msg) override;
        void receive(ClientAppend &msg) override;
        void receive(ClientDelete &msg) override;

        void execute(ClientLoad &msg) override;
        void execute(ClientList &msg) override;
        void execute(ClientAppend &msg) override;
        void execute(ClientDelete &msg) override;

    private:
        std::map<int, std::string> uids;   

    protected:
        Role role;

        int uid;        // MPI rank
        int nb_servers; // MPI size

        // Election
        size_t term = 0;
        microseconds election_timeout;
        chrono_time last_checked;
        int leader_uid;

        // Persistant state
        unsigned int current_term;
        int voted_for;

        // Volatile state
        unsigned int commit_index;
        unsigned int last_applied;

        // Volatile leader state
        std::vector<unsigned int> next_index;
        std::vector<unsigned int> match_index;
        
        // Fixed heartbeat timeout fitting the raft election timeout
        const static milliseconds heartbeat = 30ms;
    };

} // namespace raft