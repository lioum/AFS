#include "raft_server.hh"
#include "utils.hh"

#include <chrono>
#include <random>
#include <thread>

const milliseconds RaftServer::heartbeat = 30ms;

// Raft timeout is between 150 and 300 ms in the reference paper
milliseconds random_election_timeout()
{
    std::random_device rd;
    std::uniform_int_distribution<int> dist(150, 300);

    return milliseconds(dist(rd));
}

RaftServer::RaftServer(MPI_Comm com, int nb_servers,
                       const std::filesystem::path &folder_path)
    : InternProcessus(com, nb_servers, folder_path)
{
    role = Role::FOLLOWER;

    election_timeout = random_election_timeout();
    leader_uid = -1;

    current_term = 0;
    voted_for = -1;

    commit_index = 0;
    last_applied = 0;
}

void RaftServer::work()
{
    if (crashed)
    {
        return;
    }

    if (commit_index != last_applied)
    {
        logs[commit_index].command->call_execute(*this);
        commit_index++;
    }

    int duration = as_integer(speed) * as_integer(speed) * 1000;
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));

    // Update timeouts for election
    if (role != Role::LEADER)
    {
        auto time_delta = std::chrono::system_clock::now() - last_checked;
        election_timeout -=
            std::chrono::duration_cast<milliseconds>(time_delta);

        last_checked = std::chrono::system_clock::now();

        // Start new election if timeout
        if (election_timeout <= 0ms)
        {
            role = Role::CANDIDATE;
            current_term++;
            auto request = RpcRequestVote(-1, uid, current_term, last_applied,
                                          logs[last_applied].term);
            broadcast_to_servers(request);
            election_timeout = random_election_timeout();
        }
    }
}

void RaftServer::receive(RpcRequestVote &)
{
    if (crashed)
    {
        return;
    }
}

void RaftServer::receive(RpcAppendEntries &)
{
    if (crashed)
    {
        return;
    }
}

void RaftServer::receive(RpcVoteResponse &)
{
    if (crashed)
    {
        return;
    }
}

void RaftServer::receive(RpcAppendEntriesResponse &)
{
    if (crashed)
    {
        return;
    }
}

void RaftServer::receive(HandshakeFailure &)
{
    if (crashed)
    {
        return;
    }
}

void RaftServer::receive(HandshakeSuccess &)
{
    if (crashed)
    {
        return;
    }
}

void RaftServer::receive(ClientLoad &msg)
{
    if (crashed)
    {
        return;
    }

    std::cout << "RaftServer(" << uid << ") is loading file " << msg.filename
              << std::endl;
    logs.push_back(LogEntry(current_term, std::make_shared<ClientLoad>(msg)));
}

void RaftServer::receive(ClientList &msg)
{
    if (crashed)
    {
        return;
    }

    std::cout << "RaftServer(" << uid << ") is going to list all files"
              << std::endl;
    logs.push_back(LogEntry(current_term, std::make_shared<ClientList>(msg)));
}

void RaftServer::receive(ClientAppend &msg)
{
    if (crashed)
    {
        return;
    }

    std::cout << "RaftServer(" << uid << ") is adding " << msg.content
              << " to file with uid " << msg.uid << std::endl;
    logs.push_back(LogEntry(current_term, std::make_shared<ClientAppend>(msg)));
}

void RaftServer::receive(ClientDelete &msg)
{
    if (crashed)
    {
        return;
    }

    std::cout << "RaftServer(" << uid << ") is deleting file with uid "
              << msg.uid << std::endl;
    logs.push_back(LogEntry(current_term, std::make_shared<ClientDelete>(msg)));
}

void RaftServer::execute(ClientLoad &msg)
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
    send(HandshakeSuccess(msg.sender_rank, uid, custom_data));
}

void RaftServer::execute(ClientList &msg)
{
    std::vector<int> list_uids;
    for (const auto &key_value : uids)
    {
        list_uids.push_back(key_value.first);
    }

    json custom_data;
    custom_data["UIDS"] = list_uids;
    send(HandshakeSuccess(msg.sender_rank, uid, custom_data));
}

void RaftServer::execute(ClientAppend &msg)
{
    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, uids[msg.uid].c_str(), MPI_MODE_APPEND,
                  MPI_INFO_NULL, &file);
    MPI_File_write(file, msg.content.c_str(), msg.content.size(), MPI_CHAR,
                   MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    send(HandshakeSuccess(msg.sender_rank, uid));
}

void RaftServer::execute(ClientDelete &msg)
{
    MPI_File_delete(uids[msg.uid].c_str(), MPI_INFO_NULL);
    uids.erase(msg.uid);

    send(HandshakeSuccess(msg.sender_rank, uid));
}