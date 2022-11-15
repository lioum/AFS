#include "raft_server.hh"

#include <chrono>
#include <thread>

#include "client_message.hh"
#include "handshake_message.hh"
#include "repl_message.hh"
#include "utils.hh"

Client::Client(MPI_Comm com, int nb_servers) : Processus(com)
        , private_folder_location("client_files/client_" + state.get_rank() + "/")
{
  // Read commands from file and load them to the queue;

  MPI_File file_tmp;

  MPI_File_open(MPI_COMM_SELF, "bit.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file_tmp);

  MPI_File_write(file_tmp, "byte", 4, MPI_CHAR, MPI_STATUS_IGNORE);

  MPI_File_close(&file_tmp);

  std::string filename = private_folder_location + "commands_" + std::to_string(state.get_rank()) + ".txt";
  int size = 1000;
  char *buf = (char *)calloc(size, sizeof(char));
  MPI_File file;

  int open_failed = MPI_File_open(MPI_COMM_SELF, filename.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
  if (open_failed)
  {
    std::cout << "Failed to open: " << filename << std::endl;
  }
  MPI_File_read_all(file, buf, size, MPI_CHAR, MPI_STATUS_IGNORE);
  std::stringstream stream = chars_to_stream(buf, size);

  std::string line;
  while (std::getline(stream, line))
  {

    std::vector<std::string> inline_words = split_words(line, ' ');
    target_rank = 0;
    int sender_rank = state.get_rank();

    // Get target rank
    // Looking for living target
    while (!target_alive)
    {
      target_rank += 1;
      std::cout << "target rank: " << target_rank << std::endl;
      // Test if target server answers
      std::shared_ptr<message::Message> test_message = std::make_shared<message::Client_message>(message::ClientAction::LOAD, target_rank, sender_rank, "", "");
      ;
      send(target_rank, test_message);
    }

    std::cout << "exit while with target_alive: " << target_rank << std::endl;

    if (inline_words[0] == "LOAD")
    {
      if (inline_words.size() < 2)
      {
        std::cout << "Filename required after LOAD in file: " << filename << std::endl;
        exit(0);
      }

      // Read file content to content
      std::string content = readFileIntoString(filename);

      action_queue.push_back([this, content, inline_words]() {
        std::shared_ptr<message::Message> message = std::make_shared<message::Client_message>(message::ClientAction::LOAD, target_rank, state.get_rank(), inline_words[1], content);
        send(target_rank, message);
      });
    }
    else if (inline_words[0] == "LIST")
    {
      if (inline_words.size() > 1)
      {
        std::cout << "No text required after LIST in file: " << filename << std::endl;
        exit(0);
      }
      action_queue.push_back([this]() {
        std::shared_ptr<message::Message> message = std::make_shared<message::Client_message>(message::ClientAction::LIST, target_rank, state.get_rank(), "", "");
        send(target_rank, message);
      });
    }
    else if (inline_words[0] == "APPEND")
    {
      if (inline_words.size() < 3)
      {
        std::cout << "filename and content required after APPEND in file: " << filename << std::endl;
        exit(0);
      }
      action_queue.push_back([this, inline_words]() {
        std::shared_ptr<message::Message> message = std::make_shared<message::Client_message>(message::ClientAction::APPEND, target_rank, state.get_rank(), inline_words[1], inline_words[2]);
        send(target_rank, message);
      });
    }
    else if (inline_words[0] == "DELETE")
    {
      if (inline_words.size() < 2)
      {
        std::cout << "Filename required after DELETE in file: " << filename << std::endl;
        exit(0);
      }
      action_queue.push_back([this, inline_words]() {
        std::shared_ptr<message::Message> message = std::make_shared<message::Client_message>(message::ClientAction::DELETE, target_rank, state.get_rank(), inline_words[1], "");
        send(target_rank, message);
      });
    }
  }
}

void Client::work()
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
}

void Client::receive(HandshakeFailure &msg)
{
  if (!started || crashed)
  {
    return;
  }
  msg = msg;
}

void Client::receive(HandshakeSuccess &msg)
{
  if (!started || crashed)
  {
    return;
  }
  msg = msg;
}