#include "client_server.hh"
#include "repl_message.hh"
#include "handshake_message.hh"
#include "client_message.hh"
#include "utils.hh"

#include <thread>
#include <chrono>
#include <sstream>

namespace client {
ClientServer::ClientServer(MPI_Comm com, int nb_servers) : Server(com, nb_servers), crashed(false), started(false), speed(repl::ReplSpeed::FAST) 
{
    // Read commands from file and load them to the queue;
    std::cout << "Creating client server in clientserver.cc" << std::endl;

    MPI_File file_tmp;
        std::cout << "1" << std::endl;

    MPI_File_open(MPI_COMM_SELF, "bit.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file_tmp);
    std::cout << "2" << std::endl;

    MPI_File_write(file_tmp, "byte", 4, MPI_CHAR, MPI_STATUS_IGNORE);
    std::cout << "3" << std::endl;

    MPI_File_close(&file_tmp);
    std::cout << "End creating file in clientserver.cc" << std::endl;

    std::string filename = "commands_" + std::to_string(state.get_rank()) + ".txt";
    int size = 1000;
    char* buf = (char*) calloc(size, sizeof(char));
    MPI_File file;
    std::cout << "Start opening file in clientserver.cc" << std::endl;

    int open_failed = MPI_File_open(MPI_COMM_SELF, filename.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    std::cout << "End opening file in clientserver.cc" << std::endl;
    if (open_failed) {
      std::cout << "Failed to open: " << filename << std::endl;
    }
    std::cout << "start reading file in clientserver.cc" << std::endl;
    MPI_File_read_all(file, buf, size, MPI_CHAR, MPI_STATUS_IGNORE);
    std::cout << "End reading file in clientserver.cc" << std::endl;
    std::stringstream stream = chars_to_stream(buf, size);
    std::cout << "End reading to stream in clientserver.cc" << std::endl;

    std::string line;
    while (std::getline(stream, line)) {

      std::vector<std::string> inline_words = split_words(line, ' ');
      target_rank = 0;
      int sender_rank = state.get_rank();

      // Get target rank
      // Looking for living target
      while (!target_alive) {
        target_rank += 1;
        std::cout << "target rank: " << target_rank << std::endl;
        // Test if target server answers
        std::shared_ptr<message::Message> test_message = std::make_shared<message::Client_message>(message::ClientAction::LOAD, target_rank, sender_rank, "", "");;
        send(target_rank, test_message);
        }

      std::cout << "exit while with target_alive: " << target_rank << std::endl;


      std::shared_ptr<Client_message> message;
      if (inline_words[0] == "LOAD") {
        if (inline_words.size() < 2) {
          std::cout << "Filename required after LOAD in file: " << filename << std::endl;
          exit(0);
        }

        // Read file content to content
        std::string content = readFileIntoString(filename);

        message = std::make_shared<message::Client_message>(message::ClientAction::LOAD, target_rank, sender_rank, inline_words[0], content);
      } else if (inline_words[0] == "LIST") {
        if (inline_words.size() > 1) {
          std::cout << "No text required after LIST in file: " << filename << std::endl;
          exit(0);
        }
        message = std::make_shared<message::Client_message>(message::ClientAction::LIST, target_rank, sender_rank, "", "");
      } else if (inline_words[0] == "APPEND") {
        if (inline_words.size() < 3) {
          std::cout << "filename and content required after APPEND in file: " << filename << std::endl;
          exit(0);
        }
        message = std::make_shared<message::Client_message>(message::ClientAction::APPEND, target_rank, sender_rank, inline_words[1], inline_words[2]);
      } else if (inline_words[0] == "DELETE") {
        if (inline_words.size() < 2) {
          std::cout << "Filename required after DELETE in file: " << filename << std::endl;
          exit(0);
        }        
        message = std::make_shared<message::Client_message>(message::ClientAction::DELETE,target_rank, sender_rank, inline_words[1], "");
      }

      message_queue.push(message);
    }

}

void ClientServer::work()
{
  std::cout << "Start message callback in server.cc" << std::endl;
  if (!started || crashed)
  {
    return;
  }

  // Do the work

  // we will need to use a queue, 
  // we will pop the element from the queue and do the work
  // when a message is received in on_receive_client, we will push it to the queue
  if (!message_queue.empty())
  {
    std::shared_ptr<message::Message> message = message_queue.front();
    message_queue.pop();
    Server::send(target_rank, message);
    // do the work
  }


  std::this_thread::sleep_for(std::chrono::milliseconds(speed * speed * 1000));
}

void ClientServer::on_receive_repl(std::shared_ptr<message::Message> message) {
  std::cout << "RaftServer(" << state.get_rank() << "): Received REPL message"
            << std::endl;
  json j = message->serialize_json();
  
  json repl_message = j["REPL"];

  if (repl_message["REPL_TYPE"] == repl::ReplType::CRASH) 
  {
    std::cout << "RaftServer(" << state.get_rank()
              << ") is crashing. Bravo Six, going dark" << std::endl;
    crashed = true;
    Server::send(j["SENDER"],
         std::make_shared<message::Handshake_message>(
             message::HandshakeStatus::SUCCESS, j["SENDER"], state.get_rank()));
  }
  else if (repl_message["REPL_TYPE"] == repl::ReplType::SPEED)
  {
    repl::ReplSpeed newspeed = static_cast<repl::ReplSpeed>(j["REPL"]["SPEED"]);
    std::cout << "RaftServer(" << state.get_rank()
              << ") is changing speed from " << speed << " to "
              << j["REPL"]["SPEED"] << std::endl;
    speed = newspeed;
    Server::send(j["SENDER"],
         std::make_shared<message::Handshake_message>(
             message::HandshakeStatus::SUCCESS, j["SENDER"], state.get_rank()));
  }
  else if (repl_message["REPL_TYPE"] == repl::ReplType::START)
  {
    std::cout << "RaftServer(" << state.get_rank() << ") is starting"
              << std::endl;
    started = true;
    Server::send(j["SENDER"],
         std::make_shared<message::Handshake_message>(
             message::HandshakeStatus::SUCCESS, j["SENDER"], state.get_rank()));
  }
}

void ClientServer::on_receive_rpc(std::shared_ptr<message::Message> message) {
  json j = message->serialize_json();
  j = j;
}

void ClientServer::on_message_callback(
    std::shared_ptr<message::Message> message) {
  
  std::cout << "Start message callback in server.cc" << std::endl;
  json j = message->serialize_json();

  if (crashed && j["MESSAGE_TYPE"] != message::MessageType::REPL) {
    target_alive = false;
    return;
  }

  if (j["MESSAGE_TYPE"] == message::MessageType::REPL) {
    target_alive = true;
    on_receive_repl(message);
  }

  if (j["MESSAGE_TYPE"] == message::MessageType::RPC) {
    std::cout << "ClientServer(" << state.get_rank() << "): Received RPC message"
              << std::endl;
    target_alive = true;
    on_receive_rpc(message);
  }

  if (j["MESSAGE_TYPE"] == message::MessageType::CLIENT) {
    std::cout << "ClientServer(" << state.get_rank()
              << "): Received CLIENT message" << std::endl;
    target_alive = true;
    message_queue.push(message);
  }

  if (j["MESSAGE_TYPE"] == message::MessageType::HANDSHAKE) {
    std::cout << "ClientServer(" << state.get_rank()
              << "): Received HANDSHAKE message" << std::endl;
    target_alive = true;
    message_queue.push(message);
  }
}
} // namespace raft