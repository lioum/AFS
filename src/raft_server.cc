#include "raft_server.hh"

#include "message.hh"
#include "utils.hh"

// Constructor definition
RaftServer::RaftServer(int nb_servers,
                       const std::filesystem::path &root_folder_path)
    : InternProcessus(nb_servers, root_folder_path)
    , role(Role::FOLLOWER)
    , leader_uid(-1)
    , vote_count(0)
    , current_term(0)
    , voted_for(-1)
    , commit_index(0)
    , last_applied(0)
{
    election_timeout = random_election_timeout();
    heartbeat_timeout = heartbeat;
    last_checked = std::chrono::system_clock::now();

    // Initialize entries with a fake log to avoid access checking to empty
    // entries
    entries.push_back(LogEntry(-1, nullptr));
    server_str = "[Server " + std::to_string(uid) + "]";

    for (int i = 1; i <= nb_servers; i++)
    {
        next_index.push_back(1);
        match_index.push_back(1);
    }
}

int RaftServer::get_last_log_term()
{
    return (entries.end() - 1)->term;
}

int RaftServer::get_last_log_index()
{
    return entries.size() - 1;
}

int RaftServer::get_prev_log_index(int rank)
{
    return next_index[rank - 1] - 1;
}

int RaftServer::get_prev_log_term(int rank)
{
    return entries[get_prev_log_index(rank)].term;
}

void RaftServer::save_logs()
{
    std::string filename =
        "server_folders/logs/" + std::to_string(uid) + ".logs";
    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, filename.c_str(),
                  MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);
    json j = entries;
    std::string content = j.dump();

    MPI_File_write(file, content.c_str(), content.size(), MPI_CHAR,
                   MPI_STATUS_IGNORE);

    MPI_File_close(&file);
}

void RaftServer::work()
{
    if (crashed)
        return;
    std::this_thread::sleep_for(sleeping_time);
    apply_server_rules();
}

void RaftServer::broadcast_append_entries(RpcAppendEntries &msg)
{
    for (int i = 1; i <= nb_servers; i++)
    {
        if (i != uid)
        {
            msg.prev_log_index = get_prev_log_index(i);
            msg.prev_log_term = get_prev_log_term(i);
            msg.target_rank = i;
            send(msg);
        }
    }
}

void RaftServer::start_election()
{
    // New election, new term
    current_term++;
    std::cerr << server_str << " Starts an election" << std::endl;

    // All candidates have at least one vote (from themselves)
    vote_count = 1;
    voted_for = uid;

    // Send vote request to all servers
    RpcRequestVote request(-1, uid, current_term, uid, get_last_log_index(),
                           get_last_log_term());
    broadcast_to_servers(request);

    // Reset randomized election timeout
    election_timeout = random_election_timeout();
}

void RaftServer::update_timeouts()
{
    nanoseconds time_delta = std::chrono::system_clock::now() - last_checked;
    if (role != Role::LEADER)
        election_timeout = election_timeout - (time_delta - sleeping_time);
    else
        heartbeat_timeout = heartbeat_timeout - (time_delta - sleeping_time);
    last_checked = std::chrono::system_clock::now();
}

void RaftServer::apply_server_rules()
{
    // If the entry has been safetly replicated
    //      => leader applies the entry to its state machine
    //      => available state machine executes the same command in the same
    //      order
    // Saftely = the newly created entry has been replicated on a majority
    if (commit_index > last_applied)
    {
        last_applied++;
        auto command = entries[last_applied].command;
        command->call_execute(*this);
        if (role == Role::LEADER)
            std::cerr << server_str << " Executed command " << last_applied + 1
                      << " which came from client " << command->client_uid
                      << std::endl;
    }

    update_timeouts();

    // Apply rules according to their identity
    if (role == Role::LEADER)
        apply_leader_rules();
    else
        apply_follower_and_candidate_rules();
}

void RaftServer::apply_follower_and_candidate_rules()
{
    // Start new election if timeout
    if (election_timeout <= 0ms)
    {
        role = Role::CANDIDATE;
        start_election();
    }
}

void RaftServer::apply_leader_rules()
{
    // Keep sending periodic heartbeat/AppendEntries every 30ms
    if (heartbeat_timeout <= 0ms)
    {
        for (int i = 1; i <= nb_servers; i++)
        {
            if (i != uid)
            {
                // Update resumed servers whose logs are not up-to-date
                // Or send heartbeat if they are
                std::vector<LogEntry> msg_entries(
                    entries.begin() + next_index[i - 1], entries.end());
                RpcAppendEntries to_send(
                    i, uid, current_term, leader_uid, get_prev_log_index(i),
                    get_prev_log_term(i), msg_entries, commit_index);

                send(to_send);
            }
        }
        // Reset heartbeat timeout
        heartbeat_timeout = 30ms;
    }

    // Update commit_index if the majority has been replicated
    int n = commit_index + 1;
    int count = 1;
    for (int i = 1; i <= nb_servers; i++)
    {
        if (match_index[i - 1] >= n && i != uid)
            count++;
    }
    if (2 * count > nb_servers && (size_t)n < entries.size())
        commit_index++;
}