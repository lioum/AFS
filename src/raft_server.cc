#include "raft_server.hh"
#include "repl_message.hh"
#include "handshake_message.hh"

namespace raft {
RaftServer::RaftServer(MPI_Comm com) : Server(com), crashed(false) {}

void RaftServer::work()
{

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
  }

  if (j["MESSAGE_TYPE"] == message::MessageType::RPC) {
    {
      std::cout << "RaftServer(" << state.get_rank()
                << "): Received RPC message" << std::endl;
    }
  }
}
} // namespace raft