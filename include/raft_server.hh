#pragma once

#include <mpi.h>
#include <queue>

#include "processus.hh"

using chrono_time = std::chrono::time_point<std::chrono::system_clock>;
using namespace std::chrono_literals;

/*
** Enum of possible role for a RaftServer
*/
enum class Role
{
    CANDIDATE,
    FOLLOWER,
    LEADER
};

/*
** Class RaftServer
**
** Instance of an InternProcessus
**
*/
class RaftServer : public InternProcessus
{
public:
    /*
    ** Constructor
    **
    ** @MPI_Comm com : the communicator provided by MPI that contains all MPI processes
    ** @int nb_servers : the number of servers
    ** @std::filesystem::path &folder_path : the reference to the folder of the processus
    */
    RaftServer(MPI_Comm com, int nb_servers, const std::filesystem::path& folder_path);

    void work() override;

    void receive(RpcMessage &msg) override;
    void receive(RpcRequestVote &msg) override;
    void receive(RpcAppendEntries &msg) override;
    void receive(RpcVoteResponse &msg) override;
    void receive(RpcAppendEntriesResponse &msg) override;

    void receive(HandshakeFailure &msg) override;
    void receive(HandshakeSuccess &msg) override;

    void receive(ClientRequest &msg) override;

    virtual void execute(Delete &) override;
    virtual void execute(Load &) override;
    virtual void execute(Append &) override;
    virtual void execute(List &) override;
    virtual void execute(ClientDelete &) override;

    /*
    ** start_election Function
    **
    ** Request to all servers their vote for leader and to spread it to the others
    ** if there is a timeout, launch a random election
    */
    void start_election();
    
    /*
    ** update_timeouts Function
    **
    ** Updeta the timeouts of the servers accordly to their role
    ** A follower will candidate if not in timeouts
    ** A leader will give a heartbeat
    */
    void update_timeouts();
    
    /*
    ** apply_server_rules Function
    **
    ** the leader will spread his 'authority' over others servers if he isnt timeout
    */   
    void apply_server_rules();
    
    /*
    ** apply_follower_and_candidate_rules Function
    **
    ** apply update_timeouts and check if a re-selection need to occur in which case the followers not timeouts will candidate
    */   
    void apply_follower_and_candidate_rules();
    
    /*
    ** aplly_leader_rules Function
    **
    ** Check if the server is the leader and give him autority accordly to his role 
    */   
    void apply_leader_rules();

    // Fixed heartbeat timeout fitting the raft election timeout
private:
    std::map<int, std::string> uids;

protected:
    Role role;

    std::vector<LogEntry> entries;

    // Election
    nanoseconds election_timeout;
    chrono_time last_checked;
    int leader_uid;
    int vote_count;

    // Persistant state
    int current_term;
    int voted_for;

    // Volatile state
    int commit_index;
    int last_applied;

    // Volatile leader state
    std::vector<int> next_index;
    std::vector<int> match_index;

    // Fixed heartbeat timeout fitting the raft election timeout
    constexpr static nanoseconds heartbeat = 30ms;
    nanoseconds heartbeat_timeout;
    chrono_time last_heartbeat;
};