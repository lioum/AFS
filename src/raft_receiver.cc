#include "raft_server.hh"
#include "utils.hh"

/*
** Functions that implements raft servers behavior when receiving a message
** Messages will be dynamically dispatched in thanks to visitor design pattern
*/

void RaftServer::receive(ClientRequest &msg)
{
    if (crashed)
        return;

    // Leader should handle all client requests
    if (role == Role::LEADER)
    {
        std::cerr << server_str << " Received request from client "
                  << msg.command->client_uid << std::endl;

        // if command received from client is unique, append entry to local log
        for (size_t i = 1; i < entries.size(); i++)
        {
            if (entries[i].command->client_uid == msg.command->client_uid
                && entries[i].command->command_id == msg.command->command_id)
            {
                std::cerr << server_str
                          << "Command already in log, no need to add it again."
                          << std::endl;
                return;
            }
        }
        entries.push_back(LogEntry(current_term, msg.command));
    }
    else // Servers whose role is not leader should redirect the request to the
         // leader
    {
        auto to_send = MeNotLeader(msg.sender_rank, uid, leader_uid);
        // redirect client to leader
        send(to_send);
    }
}

void RaftServer::receive(ReplRecovery &msg)
{
    election_timeout = random_election_timeout();
    last_checked = std::chrono::system_clock::now();
    InternProcessus::receive(msg);
}

/* ========= Receive functions for Rpc Messages ========= */

void RaftServer::receive(RpcMessage &msg)
{
    // Crashed servers cannot receive any messages
    if (crashed)
        return;

    election_timeout = random_election_timeout();

    // on receive message
    if (msg.term > current_term)
    {
        current_term = msg.term;
        voted_for = -1;
        role = Role::FOLLOWER;
    }
}

void RaftServer::receive(RpcRequestVote &msg)
{
    // Crashed servers cannot receive any messages
    if (crashed)
        return;

    if (role == Role::CANDIDATE)
    {
        if (current_term >= msg.term)
            send(RpcVoteResponse(msg.sender_rank, uid, current_term, false));
        else
            role = Role::FOLLOWER;
    }
    // if receive requestVote from candidate
    // respond to requestVote with voteResponse
    if (role == Role::FOLLOWER)
    {
        // Raft determines which of two logs is more up-to-date.
        // If the logs have last entries with different terms => the log with
        // the later term is more up-to-date If the logs end with the same term
        // => the longer log is more up-to-date
        bool me_more_up_to_date = (get_last_log_term() > msg.last_log_term)
            || (get_last_log_term() == msg.last_log_term
                && get_last_log_index() > msg.last_log_index);

        // Voter denies its vote if:
        // - it has already voted for someone else
        // - its own log is more up-to-date than that of the candidate
        if ((voted_for == -1 || voted_for == msg.candidate_uid)
            && !me_more_up_to_date)
        {
            std::cerr << server_str << " Will vote for " << msg.candidate_uid
                      << std::endl;

            voted_for = msg.candidate_uid;
            send(RpcVoteResponse(msg.sender_rank, uid, current_term, true));
        }
    }
}

void RaftServer::receive(RpcAppendEntries &msg)
{
    // Crashed servers cannot receive any messages
    if (crashed)
        return;

    if (role == Role::CANDIDATE)
        role = Role::FOLLOWER;

    if (role == Role::FOLLOWER)
    {
        // If receive appendEntries from current leader
        // respond to appendEntries with appendEntriesResponse
        leader_uid = msg.leader_uid;

        // Reply false if term < currentTerm (§5.1)
        // Reply false if log doesn’t contain an entry at prevLogIndex
        // whose term matches prevLogTerm (§5.3)
        if (msg.term < current_term || msg.prev_log_index > get_last_log_index()
            || entries[msg.prev_log_index].term != msg.prev_log_term)
        {
            send(RpcAppendEntriesResponse(msg.leader_uid, uid, current_term,
                                          false, 0));
        }
        else
        {
            if (msg.entries.size() > 0)
            {}

            // If an existing entry conflicts with a new one (same index but
            // different terms), delete the existing entry and all that follow
            // it
            // Append any new entries not already in the log
            bool found_valid_entry = false;
            int next_log_index = msg.prev_log_index + 1;
            for (size_t i = 0; i < msg.entries.size(); i++)
            {
                // std::cout << "entries[msg.prev_log_index + i].term: "
                //           << entries[msg.prev_log_index + i].term <<
                //           std::endl;
                // std::cout << "msg.entries[i].term: " << msg.entries[i].term
                //           << std::endl;
                found_valid_entry |= (next_log_index + i) >= entries.size();
                if (!found_valid_entry
                    && entries[next_log_index + i].term != msg.entries[i].term)
                {
                    // std::cout << "before erase in raft server.cc" <<
                    // std::endl;
                    entries.erase(entries.begin() + next_log_index + i,
                                  entries.end());
                    found_valid_entry = true;
                }
                if (found_valid_entry)
                    entries.push_back(msg.entries[i]);
            }

            // if leaderCommit > commitIndex, set commitIndex =
            // min(leaderCommit, index of last new entry)
            if (msg.leader_commit > commit_index)
            {
                commit_index = std::min<int>(
                    msg.leader_commit, msg.prev_log_index + msg.entries.size());
            }

            if (msg.entries.size() > 0)
            {
                std::cerr << server_str << " Received new entries: ";
                show_entries(msg.entries);
                std::cerr << std::endl
                          << server_str << " Entries after replication: ";
                show_entries(entries);
                std::cerr << std::endl;
            }

            send(RpcAppendEntriesResponse(msg.leader_uid, uid, current_term,
                                          true, get_last_log_index()));
        }
    }
}

void RaftServer::receive(RpcVoteResponse &msg)
{
    // Crashed servers cannot receive any messages
    if (crashed)
        return;

    if (role == Role::CANDIDATE)
    {
        vote_count += msg.vote_granted;

        if (vote_count > nb_servers / 2)
        {
            std::cerr << server_str << " Received at least " << vote_count
                      << " votes, he is now the leader " << std::endl;
            role = Role::LEADER;
            leader_uid = uid;
            for (int i = 0; i < nb_servers; i++)
            {
                next_index[i] = get_last_log_index() + 1;
                match_index[i] = 0;
            }

            // Send heartbeat == empty appendEntries
            auto heartbeat = RpcAppendEntries(
                -1, uid, current_term, uid, get_prev_log_index(uid),
                get_prev_log_term(uid), std::vector<LogEntry>(), commit_index);

            broadcast_to_servers(heartbeat);
        }
    }
}

void RaftServer::receive(RpcAppendEntriesResponse &msg)
{
    // Crashed servers cannot receive any messages
    if (crashed)
        return;

    // Only the leader is supposed to receive AppendEntriesResponse
    if (role == Role::LEADER)
    {
        if (get_last_log_index() < msg.match_index)
            return;

        // if receive AppendEntriesResponse from follower with success
        if (msg.success)
        {
            match_index[msg.sender_rank - 1] = msg.match_index;
            next_index[msg.sender_rank - 1] =
                match_index[msg.sender_rank - 1] + 1;
        }
        // if receive AppendEntriesResponse from follower with failure
        else
        {
            // decrement nextIndex and retry (fails due to log inconsistency)
            next_index[msg.sender_rank - 1] = std::max(
                match_index[msg.sender_rank - 1],
                next_index[msg.sender_rank - 1] - 1); // Ensure next_index >= 0

            std::vector<LogEntry> msg_entries(
                entries.begin() + next_index[msg.sender_rank - 1],
                entries.end());

            auto new_msg = RpcAppendEntries(
                msg.sender_rank, uid, current_term, uid,
                get_prev_log_index(msg.sender_rank),
                get_prev_log_term(msg.sender_rank), msg_entries, commit_index);

            send(new_msg);
        }
    }
}