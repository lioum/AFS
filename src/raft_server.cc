#include "raft_server.hh"
#include "repl_message.hh"
#include "handshake_message.hh"

#include <thread>
#include <chrono>

namespace raft {
RaftServer::RaftServer(MPI_Comm com) : Server(com), crashed(false), started(false),  speed(repl::ReplSpeed::FAST) {}

void RaftServer::work()
{
  if (!started)
  {
    return;
  }
std::this_thread::sleep_for(std::chrono::milliseconds(speed * 1000));

}

void RaftServer::on_message_callback(
    std::shared_ptr<message::Message> message) {
  json j = message->serialize_json();

  if (crashed && j["MESSAGE_TYPE"] != message::MessageType::REPL) {
    return;
  }

  if (j["MESSAGE_TYPE"] == message::MessageType::REPL) {
    std::cout << "RaftServer(" << state.get_rank() << "): Received REPL message"
              << std::endl;
    json repl_message = j["REPL"];
    if (repl_message["REPL_TYPE"] == repl::ReplType::CRASH) {
      std::cout << "RaftServer(" << state.get_rank()
                << ") is crashing. Bravo Six, going dark" << std::endl;
      crashed = true;
      send(j["SENDER"], std::make_shared<message::Handshake_message>( message::HandshakeStatus::SUCCESS, j["SENDER"], state.get_rank()));
    }
    else if (repl_message["REPL_TYPE"] == repl::ReplType::SPEED)
    {
      repl::ReplSpeed newspeed = static_cast<repl::ReplSpeed>(j["REPL"]["SPEED"]);
      std::cout << "RaftServer(" << state.get_rank() << ") is changing speed from " << speed << " to " << j["REPL"]["SPEED"] << std::endl;
      speed = newspeed;
      send(j["SENDER"], std::make_shared<message::Handshake_message>( message::HandshakeStatus::SUCCESS, j["SENDER"], state.get_rank()));
    }
    else if (repl_message["REPL_TYPE"] == repl::ReplType::START)
    {
      std::cout << "RaftServer(" << state.get_rank() << ") is starting" << std::endl;
      started = true;
      send(j["SENDER"], std::make_shared<message::Handshake_message>( message::HandshakeStatus::SUCCESS, j["SENDER"], state.get_rank()));
    }
  }

  if (j["MESSAGE_TYPE"] == message::MessageType::RPC) {
    {
      std::cout << "RaftServer(" << state.get_rank()
                << "): Received RPC message" << std::endl;
    }
  }
}
} // namespace raft