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
        , crashed(false)
        , started(false)
        , speed(repl::ReplSpeed::FAST)
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

        // Do the work

        // we will need to use a queue,
        // we will pop the element from the queue and do the work
        // when a message is received in on_receive_client, we will push it to
        // the queue
        if (!message_queue.empty())
        {
            std::shared_ptr<message::Message> message = message_queue.front();
            message_queue.pop();
            process_message_client(message);
            // do the work
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
 
  void RaftServer::receive(ReplCrash &msg)
  {
    std::cout << "RaftServer(" << state.get_rank()
              << ") is crashing. Bravo Six, going dark" << std::endl;
              
    crashed = true;
    auto response = std::make_shared<message::HandshakeSuccess>(msg.sender_rank, state.get_rank());
    send(msg.sender_rank, response);
  }
    
  virtual void receive(ReplSpeed &msg)
  {
    std::cout << "RaftServer(" << state.get_rank()
              << ") is changing speed from " << speed << " to "
              << msg.speed << std::endl;
    
    speed = msg.speed;
    auto response = std::make_shared<message::HandshakeSuccess>(msg.sender_rank, state.get_rank());
    send(msg.sender_rank, response);
  }
  
  virtual void receive(ReplStart &msg)
  {
    std::cout << "RaftServer(" << state.get_rank() << ") is starting" << std::endl;

    started = true;
    auto response = std::make_shared<message::HandshakeSuccess>(msg.sender_rank, state.get_rank());
    send(msg.sender_rank, response);
  }

  /*void RaftServer::on_receive_repl(std::shared_ptr<message::Message> message)
  {
      std::cout << "RaftServer(" << state.get_rank()
                << "): Received REPL message" << std::endl;
      json j = message->serialize_json();

      json repl_message = j["REPL"];

      if (repl_message["REPL_TYPE"] == repl::ReplType::CRASH)
      {
          std::cout << "RaftServer(" << state.get_rank()
                    << ") is crashing. Bravo Six, going dark" << std::endl;
          crashed = true;
          send(j["SENDER"],
               );
      }
      else if (repl_message["REPL_TYPE"] == repl::ReplType::SPEED)
      {
          repl::ReplSpeed newspeed =
              static_cast<repl::ReplSpeed>(j["REPL"]["SPEED"]);
          std::cout << "RaftServer(" << state.get_rank()
                    << ") is changing speed from " << speed << " to "
                    << j["REPL"]["SPEED"] << std::endl;
          speed = newspeed;
          send(j["SENDER"],
               std::make_shared<message::Handshake_message>(
                   message::HandshakeStatus::SUCCESS, j["SENDER"],
                   state.get_rank()));
      }
      else if (repl_message["REPL_TYPE"] == repl::ReplType::START)
      {
          std::cout << "RaftServer(" << state.get_rank() << ") is starting"
                    << std::endl;
          started = true;
          send(j["SENDER"],
               std::make_shared<message::Handshake_message>(
                   message::HandshakeStatus::SUCCESS, j["SENDER"],
                   state.get_rank()));
      }
  }*/

    void RaftServer::on_receive_rpc(std::shared_ptr<RPC> message)
    {
        auto message = RPC::deserialize();
        json j = message->serialize_json();

        json rpc_message = j["RPC"];
    }

    void RaftServer::process_message_client(
        std::shared_ptr<message::Message> message)
    {
        json j = message->serialize_json();
        json client_message = j["CLIENT"];
        if (client_message["ACTION"] == message::ClientAction::LOAD)
        {
            std::cout << "RaftServer(" << state.get_rank()
                      << ") is cloading file " << client_message["FILENAME"]
                      << std::endl;

            // LOAD FILE

            std::string filename = client_message["FILENAME"];
            std::string content = client_message["SOME_TEXT"];

            MPI_File file;
            MPI_File_open(state.get_comm(), filename.c_str(),
                          MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL,
                          &file);

            MPI_File_write(file, content.c_str(), content.size(), MPI_CHAR,
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

            send(j["SENDER"],
                 std::make_shared<message::Handshake_message>(
                     message::HandshakeStatus::SUCCESS, j["SENDER"],
                     state.get_rank(), custom_data));

            // BROADCAST TO SERVERS new file
        }
        else if (client_message["ACTION"] == message::ClientAction::LIST)
        {
            std::cout << "RaftServer(" << state.get_rank()
                      << ") is going to list all files" << std::endl;

            // LIST FILES
            std::vector<int> list_uids;
            for (const auto &[key, value] : uids)
            {
                list_uids.push_back(key);
            }

            json custom_data;
            custom_data["UIDS"] = list_uids;

            send(j["SENDER"],
                 std::make_shared<message::Handshake_message>(
                     message::HandshakeStatus::SUCCESS, j["SENDER"],
                     state.get_rank(), custom_data));
        }
        else if (client_message["ACTION"] == message::ClientAction::APPEND)
        {
            std::cout << "RaftServer(" << state.get_rank() << ") is adding "
                      << client_message["SOME_TEXT"] << " to file with uid "
                      << client_message["UID"] << std::endl;

            // APPEND TO FILE
            int uid = client_message["UID"];
            std::string content = client_message["SOME_TEXT"];

            MPI_File file;
            MPI_File_open(state.get_comm(), uids[uid].c_str(), MPI_MODE_APPEND,
                          MPI_INFO_NULL, &file);

            MPI_File_write(file, content.c_str(), content.size(), MPI_CHAR,
                           MPI_STATUS_IGNORE);

            MPI_File_close(&file);

            send(j["SENDER"],
                 std::make_shared<message::Handshake_message>(
                     message::HandshakeStatus::SUCCESS, j["SENDER"],
                     state.get_rank()));

            // BROADCAST TO SERVERS file update
        }
        else if (client_message["ACTION"] == message::ClientAction::DELETE)
        {
            std::cout << "RaftServer(" << state.get_rank()
                      << ") is deleting file with uid " << client_message["UID"]
                      << std::endl;

            // delete file
            MPI_File_delete(uids[client_message["UID"]].c_str(), MPI_INFO_NULL);
            uids.erase(client_message["UID"]);

            send(j["SENDER"],
                 std::make_shared<message::Handshake_message>(
                     message::HandshakeStatus::SUCCESS, j["SENDER"],
                     state.get_rank()));

            // BROADCAST TO SERVERS file deleted
        }
    }

    void
    RaftServer::on_message_callback(std::shared_ptr<message::Message> message)
    {
        json j = message->serialize_json();

        if (crashed && j["MESSAGE_TYPE"] != message::MessageType::REPL)
        {
            return;
        }

        if (j["MESSAGE_TYPE"] == message::MessageType::REPL)
        {
            on_receive_repl(message);
        }

        if (j["MESSAGE_TYPE"] == message::MessageType::RPC)
        {
            std::cout << "RaftServer(" << state.get_rank()
                      << "): Received RPC message" << std::endl;
            on_receive_rpc(message);
        }

        if (j["MESSAGE_TYPE"] == message::MessageType::CLIENT)
        {
            std::cout << "RaftServer(" << state.get_rank()
                      << "): Received CLIENT message" << std::endl;
            message_queue.push(message);
        }
    }
    void
    RaftServer::broadcast_to_servers(std::shared_ptr<message::Message> message)
    {
        // LOG ACTION TO LOGFILE (useful in order to recover changes for crashed
        // server coming back online)

        for (int i = 1; i <= state.get_nb_servers(); i++)
        {
            if (i != state.get_rank())
                send(i, message);
        }
    }
    

  std::shared_ptr<RPC> RaftServer::request_vote_rpc() {
      auto msg = std::make_shared<RequestVoteRPC>(state.term, state.rank, state.last_log_index, state.last_log_term);
      
      broadcast_to_servers(msg);
      
      return msg;
  };
    
} // namespace raft