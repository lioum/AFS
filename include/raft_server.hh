#pragma once

#include <mpi.h>
#include <queue>

#include "processus.hh"

using chrono_time = std::chrono::time_point<std::chrono::system_clock>;
using std::chrono::milliseconds;
using namespace std::chrono_literals;

enum class Role
{
    CANDIDATE,
    FOLLOWER,
    LEADER
};

class RaftServer : public InternProcessus
{
public:
    RaftServer(MPI_Comm com, int nb_servers, const std::filesystem::path& folder_path);

    void work() override;

    using InternProcessus::receive;
    void receive(RpcMessage &msg) override;
    void receive(RpcRequestVote &msg) override;
    void receive(RpcAppendEntries &msg) override;
    void receive(RpcVoteResponse &msg) override;
    void receive(RpcAppendEntriesResponse &msg) override;

    void receive(HandshakeFailure &msg) override;
    void receive(HandshakeSuccess &msg) override;

    void receive(ClientRequest &msg) override;
    void receive(ClientLoad &msg) override;
    void receive(ClientList &msg) override;
    void receive(ClientAppend &msg) override;
    void receive(ClientDelete &) override;

    void execute(ClientLoad &msg) override;
    void execute(ClientList &msg) override;
    void execute(ClientAppend &msg) override;
    void execute(ClientDelete &msg) override;

    void start_election();
    void update_timeouts();
    void apply_server_rules();
    void apply_follower_and_candidate_rules();
    void apply_leader_rules();

    // Fixed heartbeat timeout fitting the raft election timeout
private:
    std::map<int, std::string> uids;

protected:
    Role role;

    std::vector<LogEntry> entries;

    // Election
    milliseconds election_timeout;
    chrono_time last_checked;
    unsigned int leader_uid;
    int vote_count;

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
    constexpr static milliseconds heartbeat = 30ms;
    milliseconds heartbeat_timeout;
    chrono_time last_heartbeat;
};