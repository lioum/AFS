#pragma once

#include <thread>

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
    ** @int nb_servers : the number of servers
    ** @std::filesystem::path &folder_path : the reference to the folder of the processus
    */
    RaftServer(int nb_servers, const std::filesystem::path& root_folder_path);
    virtual ~RaftServer();

    /*
    ** Overriding work Function
    **
    ** the objective is to send the message to clients or servers 
    */
    void work() override;

    /*
    ** broadcast_append_entries Function
    **
    ** @Message & msg : reference to the message to be broadcasted
    ** Send the message to all the servers
    */
    void broadcast_append_entries(RpcAppendEntries &msg);

    /*
    ** Overriding receive Functions
    **
    ** the objective is to receive a message of a specific type
    */
    void receive(RpcMessage &msg) override;
    void receive(RpcRequestVote &msg) override;
    void receive(RpcAppendEntries &msg) override;
    void receive(RpcVoteResponse &msg) override;
    void receive(RpcAppendEntriesResponse &msg) override;
    void receive(ClientRequest &msg) override;
    void receive(ReplRecovery &msg) override;

   /*
    ** Command Execution Functions
    **
    ** the objective is to apply client request according to its specific command type
    */
    void execute(Command &msg);
    void execute(Delete &msg);
    void execute(Load &msg);
    void execute(Append &msg);
    void execute(List &msg);

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
    ** the leader will spread his 'authority' over others servers if he isn't timeout
    */   
    void apply_server_rules();
    
    /*
    ** apply_follower_and_candidate_rules Function
    **
    ** apply update_timeouts and check if a re-selection need to occur in which case the followers not timeouts will candidate
    */   
    void apply_follower_and_candidate_rules();
    
    /*
    ** apply_leader_rules Function
    **
    ** Check if the server is the leader and give him autority accordly to his role 
    */   
    void apply_leader_rules();

private:
    // std::map<int, std::string> uids;
    std::map<std::string, MPI_File> file_map;

protected:
    // Utils fonctions to get last log index and term
    int get_last_log_index();
    int get_last_log_term();
    // Utils fonctions for the leader to get followers last log index and term 
    int get_prev_log_index(int rank);
    int get_prev_log_term(int rank);

    void save_logs();
    
    Role role;

    std::vector<LogEntry> entries;
    std::string server_str;

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

    // Periodic heartbeat
    constexpr static nanoseconds heartbeat = 30ms;
    nanoseconds heartbeat_timeout;
};
