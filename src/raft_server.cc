#include "raft_server.hh"

#include <chrono>
#include <thread>

#include "client_message.hh"
#include "handshake_message.hh"
#include "repl_message.hh"

namespace raft
{
  // Raft timeout is between 150 and 300 ms in the reference paper
  milliseconds random_election_timeout()
  {
      float rand_float = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      return milliseconds(150 + rand_float * 150);
  }

  RaftServer::RaftServer(MPI_Comm com)
      : Server(com)
      , private_folder_location("server_files/server_" + state.get_rank() + "/")
  {
      MPI_Comm_rank(com, &uid);
      MPI_Comm_size(com, &nb_servers);
      
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
      if (!started || crashed)
      {
          return;
      }
            
      if (!action_queue.empty())
      {
          action_queue.front()();
          action_queue.pop_front();
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(speed * speed * 1000));
      
      // Update timeouts for election
      if (role != Role::LEADER)
      {
        auto time_delta = std::chrono::system_clock::now() - last_checked;
        election_timeout -= std::chrono::duration_cast<milliseconds>(time_delta);

        last_checked = std::chrono::system_clock::now();

        // Start new election if timeout
        if (election_timeout <= 0ms)
        {
          role = Role::CANDIDATE;
          current_term++;
          broadcast_to_servers(std::make_shared<message::RequestVote>(current_term, uid, last_log_index(), last_log_term()));
          timeout = random_election_timeout();
        }
      }
  }
 
  void RaftServer::receive(RpcRequestVote &msg) {
    if (!started || crashed) {
      return;
    }
    msg = msg;
  }

  void RaftServer::receive(RpcAppendEntries &msg) {
    if (!started || crashed) {
      return;
    }
    msg = msg;
  }

  void RaftServer::receive(RpcVoteResponse &msg) {
    if (!started || crashed) {
      return;
    }
    msg = msg;
  }

  void RaftServer::receive(RpcAppendEntriesResponse &msg) {
    if (!started || crashed) {
      return;
    }
    msg = msg;
  }

  void RaftServer::receive(HandshakeFailure &msg) {
    if (!started || crashed) {
      return;
    }
    msg = msg;
  }

  void RaftServer::receive(HandshakeSuccess &msg) {
    if (!started || crashed) {
      return;
    }
    msg = msg;
  }

  void RaftServer::receive(ClientLoad &msg)
  {
    if (!started || crashed)
    {
      return;
    }
    std::cout << "RaftServer(" << state.get_rank()
              << ") is cloading file " << msg.filename << std::endl;
    action_queue.push_back(msg);
  }

  void RaftServer::receive(ClientList &msg)
  {
    if (!started || crashed)
    {
      return;
    }

    std::cout << "RaftServer(" << state.get_rank()
              << ") is going to list all files" << std::endl;
    action_queue.push_back(msg);
  }

  void RaftServer::receive(ClientAppend &msg)
  {
    if (!started || crashed)
    {
      return;
    }

    std::cout << "RaftServer(" << state.get_rank() << ") is adding "
     << msg.content << " to file with uid "  << msg.uid << std::endl;
    action_queue.push_back(msg);
  }

  void RaftServer::receive(ClientDelete &msg)
  {
    if (!started || crashed)
    {
      return;
    }

    std::cout << "RaftServer(" << state.get_rank() << 
      ") is deleting file with uid " << msg.uid << std::endl;
    action_queue.push_back(msg);
  }
      
  void RaftServer::execute(ClientLoad &msg) override
  {
    std::string filename = private_folder_location + msg.filename;

    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);
    MPI_File_write(file, msg.content.c_str(), msg.content.size(), MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    int uid = 0;
    while (uids.find(uid) != uids.end())
    {
      uid++;
    }
    uids[uid] = filename;

    json custom_data;
    custom_data["UID"] = uid;
    handshake_success(msg.sender_rank, custom_data);
  }
  
  void RaftServer::execute(ClientList &msg) override
  { 
    std::vector<int> list_uids;
    for (const auto &[key, value] : uids)
    {
      list_uids.push_back(key);
    }

    json custom_data;
    custom_data["UIDS"] = list_uids;
    handshake_success(msg.sender_rank, custom_data);
  }

  void RaftServer::execute(ClientAppend &msg) override
  {
    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, uids[msg.uid].c_str(), MPI_MODE_APPEND, MPI_INFO_NULL, &file);
    MPI_File_write(file, msg.content.c_str(), msg.content.size(), MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    handshake_success(msg.sender_rank);
  }

  void RaftServer::execute(ClientDelete &msg) override
  {
    MPI_File file;
    MPI_File_delete(uids[msg.uid].c_str(), MPI_INFO_NULL);
    uids.erase(msg.uid);

    handshake_success(msg.sender_rank);
  }
}