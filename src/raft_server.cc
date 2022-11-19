#include "raft_server.hh"
#include "utils.hh"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <thread>

#include "message.hh"

// const milliseconds RaftServer::heartbeat = 30ms;

// Raft timeout is between 150 and 300 ms in the reference paper
nanoseconds random_election_timeout()
{
    std::random_device rd;
    std::uniform_int_distribution<long int> dist(150 * 10e5, 300 * 10e5);

    return nanoseconds(dist(rd));
}

RaftServer::RaftServer(MPI_Comm com, int nb_servers,
                       const std::filesystem::path &folder_path)
    : InternProcessus(com, nb_servers, folder_path), role(Role::FOLLOWER)
{
    election_timeout = random_election_timeout();
    std::cout << "Initial election timeout is " << std::chrono::duration_cast<milliseconds>(election_timeout) << std::endl;

    last_heartbeat = std::chrono::system_clock::now();
    last_checked = std::chrono::system_clock::now();

    // Faux log pour eviter d'avoir des checks sur les entries vides
    entries.push_back(LogEntry(-1, nullptr));

    leader_uid = -1;
    current_term = 0;
    voted_for = -1;

    commit_index = 0;
    last_applied = 0;

    vote_count = 0;
}

int RaftServer::get_last_log_term()
{
    return (entries.end() - 1)->term;
}

int RaftServer::get_last_log_index()
{
    return entries.size() - 1;
}

void RaftServer::work()
{
    std::this_thread::sleep_for(sleeping_time);
    // std::cout << std::endl << "NEW WORK LOOP:" << std::endl;
    apply_server_rules();
}

void RaftServer::apply_server_rules()
{
    if (role == Role::LEADER)
        apply_leader_rules();
    else
        apply_follower_and_candidate_rules();
}

void RaftServer::start_election()
{
    current_term++;
    std::cout << std::endl << "Starting election (" << uid << ") " << "my new term is " << current_term << std::endl;

    vote_count = 0;
    voted_for = uid;
    RpcRequestVote request (-1, uid, current_term, uid,
                            get_last_log_index(), get_last_log_term());
    std::cout << "I am sending this message: " << request.serialize() << std::endl;
    broadcast_to_servers(request);
    election_timeout = random_election_timeout();
}

void RaftServer::update_timeouts()
{
    if (role != Role::LEADER)
    {
        // Update timeouts for election
        std::chrono::nanoseconds time_delta = std::chrono::system_clock::now() - last_checked;
        /*std::cout << "time delta in update_timeout(): " 
        << std::chrono::duration_cast<microseconds>(time_delta - sleeping_time) 
        << std::endl;*/

        election_timeout = election_timeout - (time_delta - sleeping_time);

        last_checked = std::chrono::system_clock::now();
    }
    else
    {
        // Update timeouts for heartbeat
        auto time_delta = std::chrono::system_clock::now() - last_heartbeat;
        heartbeat_timeout = heartbeat_timeout - (time_delta - sleeping_time);

        last_heartbeat = std::chrono::system_clock::now();
    }
}

void RaftServer::apply_follower_and_candidate_rules()
{
    //std::cout << "Before updata: My election timeout is " << std::chrono::duration_cast<milliseconds>(election_timeout) << std::endl;
    update_timeouts();
    //std::cout << "After updata: My election timeout is " << std::chrono::duration_cast<milliseconds>(election_timeout) << std::endl;

    // Start new election if timeout    
    if (election_timeout <= 0ms)
    {
        role = Role::CANDIDATE;
        start_election();
    }
}

void RaftServer::apply_leader_rules()
{
    // send heartbeat every 30ms ~
    if (heartbeat_timeout <= 0ms)
    {
        // empty message to keep connection alive
        auto msg = RpcAppendEntries(-1, uid, current_term, uid, last_applied,
                                    entries[last_applied].term,
                                    std::vector<LogEntry>(), commit_index);

        broadcast_to_servers(msg);
    }

    //
    // if there exists an N such that N > commitIndex, a majority of
    // matchIndex[i] >= N, and log[N].term == currentTerm: MES COUILLES AU BORD
    // DE L EAU, ça fait un radeau.
    // TODO
    // Code Nathan
    // let mut match_indices = leader.match_index.clone();
    //                 match_indices[self.id] = self.entries.len();
    //                 match_indices.sort();
    //                 let new_commit_index = match_indices[self.senders.len() /
    //                 2];

    //                 if new_commit_index > self.state.commit_index
    //                     && self.entries[new_commit_index - 1].term ==
    //                     self.current_term
    //                 {
    //                     self.state.commit_index = new_commit_index;
    //                 }
    int n = commit_index + 1;
    int count = 0;
    for (int i = 1; i < nb_servers; i++)
    {
        if (match_index[i] >= n)
            count++;
    }
    if (2 * count > nb_servers && (size_t)n < entries.size())
    {
        commit_index = n;
    }
}

void RaftServer::receive(ClientRequest &msg)
{
    msg = msg; // GIGA SUS
    if (role == Role::LEADER)
    {
        // if command received from client
        // append entry to local log
        // respond after entry applied to state machine
        for (int i = 1; i < nb_servers; i++)
        {
            if (i != uid)
            {
                if (last_applied >= next_index[uid])
                {
                    auto to_send = RpcAppendEntries(i, uid,
                                                current_term, uid, next_index[i], current_term,
                                                entries, commit_index);

                    send(to_send);
                }
            }
        }
    }
    else
    {
        auto to_send = MeNotLeader(msg.sender_rank, uid, leader_uid);
        // redirect client to leader
        send(to_send);
    }
}

void RaftServer::receive(RpcMessage &msg)
{
    if (crashed)
        return;

    election_timeout = random_election_timeout();

    // on receive message
    if (commit_index > last_applied)
    {
        last_applied++;
        // apply log[last_applied] to state machine
        //log[last_applied]->execute();

        // update entries
        // check log matching?
    }
    if (msg.term > current_term)
    {
        current_term = msg.term;
        voted_for = -1;
        role = Role::FOLLOWER;
    }
}

void RaftServer::receive(RpcRequestVote &msg)
{
    if (crashed)
        return;

    std::cout << std::endl << "Receive(" << uid << "): RpcRequestVote. Role: " << as_integer(role) << std::endl;
    std::cout << "My term is " << current_term 
    << ", my role is " << as_integer(role) << " the source server (" 
    << msg.sender_rank << ") term is " << msg.term << std::endl;
    std::cout << "Message: " << msg.serialize() << std::endl;

    if (role == Role::CANDIDATE)
    {
        if (current_term >= msg.term)
        {
            std::cout << "I received a vote request but I am an up to date candidate" << std::endl;
            send(RpcVoteResponse(msg.sender_rank, uid, current_term, false));
        }
        else
        {
            std::cout << "I passed from candidate to follower";
            role = Role::FOLLOWER;
        }
    }
    // if receive requestVote from candidate
    // respond to requestVote with voteResponse
    if (role == Role::FOLLOWER)
    {
        bool up_to_date = (get_last_log_term() > msg.last_log_term) || 
                          (get_last_log_term() == msg.last_log_term 
                           && get_last_log_index() >= msg.last_log_index);

        if ((voted_for == -1 || voted_for == msg.candidate_uid) && up_to_date)
        {
            std::cout << "I will vote for him" << std::endl;
  
            voted_for = msg.candidate_uid;
            send(RpcVoteResponse(msg.sender_rank, uid, current_term, true));
        }
    }
}

void RaftServer::receive(RpcAppendEntries &msg)
{
    if (crashed)
        return;

    if (role == Role::CANDIDATE)
        role = Role::FOLLOWER;

    if (role == Role::FOLLOWER)
    {
        // if receive appendEntries from current leader
        // respond to appendEntries with appendEntriesResponse
        if (entries[msg.prev_log_index].term != msg.prev_log_term)
        {
            send(RpcAppendEntriesResponse(msg.leader_uid, uid, current_term, false));
        }
        else
        {
            // if an existing entry conflicts with a new one (same index but
            // different terms), delete the existing entry and all that follow
            // it append any new entries not already in the log
            for (size_t i = 0; i < msg.entries.size(); i++)
            {
                if (entries[msg.prev_log_index + i].term != msg.entries[i].term)
                {
                    entries.erase(entries.begin() + msg.prev_log_index + i,
                                  entries.end());
                    entries.push_back(msg.entries[i]);
                }
            }
            // if leaderCommit > commitIndex, set commitIndex =
            // min(leaderCommit, index of last new entry)
            if (msg.leader_commit > commit_index)
            {
                commit_index = std::min<int>(
                    msg.leader_commit, msg.prev_log_index + msg.entries.size());
            }
            // Heartbeat taken in account here
            send(RpcAppendEntriesResponse(msg.leader_uid, uid, current_term, true));
        }
    }
}

void RaftServer::receive(RpcVoteResponse &msg)
{
    if (crashed)
        return;

    if (role == Role::CANDIDATE)
    {
        vote_count += msg.vote_granted;
        std::cout << "I(" << uid << ") received a " << msg.vote_granted
                  << " vote response" << std::endl;

        if (vote_count > nb_servers / 2)
        {
            role = Role::LEADER;
            leader_uid = uid;
            std::cout << "I am the leader(" << uid << ") : STOP THE COUNT!"
                      << std::endl;
            // send heartbeat == empty appendEntries
            auto new_msg = RpcAppendEntries(-1, uid, current_term,
                                            uid, 0, current_term, std::vector<LogEntry>(),
                                            commit_index);

            broadcast_to_servers(new_msg);
            std::cout << "I, the new leader, didn't segfault ... maybe ?" 
            << std::endl;
        }
    }
}

void RaftServer::receive(RpcAppendEntriesResponse &msg)
{
    if (crashed)
        return;

    // Only the leader is supposed to receive AppendEntriesResponse
    if (role == Role::LEADER)
    {
        // if receive AppendEntriesResponse from follower with success
        if (msg.success)
        {
            match_index[msg.sender_rank] += 1;
            next_index[msg.sender_rank] = match_index[msg.sender_rank] + 1;
        }
        // if receive AppendEntriesResponse from follower with failure
        else
        {
            // decrement nextIndex and retry
            next_index[msg.sender_rank]--; // need to verify next_index value is
                                           // not 0 since    int
                                           // TODO decrement only if fails due
                                           // to log inconsistency (after a new
                                           // election)
            auto new_msg = RpcAppendEntries(msg.sender_rank, uid,
                                            current_term, uid, next_index[msg.sender_rank], current_term,
                                            entries, commit_index);

            send(new_msg);
        }
    }
    else
    {
        std::cerr
            << "I am not the leader, I should not receive this message from("
            << msg.sender_rank << ")" << std::endl;
    }
}

void RaftServer::receive(HandshakeFailure &)
{
    if (crashed)
        return;
}

void RaftServer::receive(HandshakeSuccess &)
{
    if (crashed)
        return;
}

// void RaftServer::receive(ClientLoad &msg)
// {
//   if (crashed)
//       return;
//   std::cout << "RaftServer(" << uid << ") is loading file " << msg.filename
//             << std::endl;
//   entries.push_back(LogEntry(current_term, msg.serialize(), msg.sender_rank));
// }

// void RaftServer::receive(ClientList &msg)
// {
//   if (crashed)
//       return;

//   std::cout << "RaftServer(" << uid << ") is going to list all files"
//             << std::endl;
//   entries.push_back(LogEntry(current_term, msg.serialize(), msg.sender_rank));
// }

// void RaftServer::receive(ClientAppend &msg)
// {
//   if (crashed)
//       return;

//   std::cout << "RaftServer(" << uid << ") is adding " << msg.content
//             << " to file with uid " << msg.uid << std::endl;
//   entries.push_back(LogEntry(current_term, msg.serialize(), msg.sender_rank));
// }

// void RaftServer::receive(ClientDelete &msg)
// {
//   if (crashed)
//       return;

// }

void RaftServer::execute(Load &msg)
{
    std::string filename = working_folder_path / msg.filename;

    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, filename.c_str(),
                  MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);
    MPI_File_write(file, msg.content.c_str(), msg.content.size(), MPI_CHAR,
                   MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    int uid = 0;
    while (uids.find(uid) != uids.end())
    {
        uid++;
    }
    uids[uid] = filename;

  json custom_data;
  custom_data["UID"] = uid;
  send(HandshakeSuccess(msg.client_uid, uid, custom_data));
}

void RaftServer::execute(List &msg)
{
    std::vector<int> list_uids;
    for (const auto &key_value : uids)
    {
        list_uids.push_back(key_value.first);
    }

  json custom_data;
  custom_data["UIDS"] = list_uids;

  std::cout << "RaftServer(" << uid << ") is going to list all files"
            << std::endl;
  //entries.push_back(LogEntry(current_term, msg));

  send(HandshakeSuccess(msg.client_uid, uid, custom_data));
}

void RaftServer::execute(Append &msg)
{
    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, uids[msg.uid].c_str(), MPI_MODE_APPEND,
                  MPI_INFO_NULL, &file);
    MPI_File_write(file, msg.content.c_str(), msg.content.size(), MPI_CHAR,
                   MPI_STATUS_IGNORE);
    MPI_File_close(&file);

  std::cout << "RaftServer(" << uid << ") is adding " << msg.content
            << " to file with uid " << msg.uid << std::endl;
  // entries.push_back(LogEntry(current_term, std::make_shared<Append>(msg)));

  send(HandshakeSuccess(msg.client_uid, uid));
}

void RaftServer::execute(Delete &msg)
{
  MPI_File_delete(uids[msg.uid].c_str(), MPI_INFO_NULL);
  uids.erase(msg.uid);

  std::cout << "RaftServer(" << uid << ") is deleting file with uid "
            << msg.uid << std::endl;
  // entries.push_back(LogEntry(current_term, msg));
}

void RaftServer::execute(ClientDelete &)
{
    throw std::runtime_error("RaftServer::execute(ClientDelete &) Not implemented");
}
