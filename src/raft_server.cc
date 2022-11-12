#include "raft_server.hh"
#include "repl_message.hh"
namespace raft {
RaftServer::RaftServer(MPI_Comm com) : Server(com), crashed(false) {}

void RaftServer::on_message_callback(std::shared_ptr<message::Message> message) {
  json j = message->serialize_json();
  
  if (crashed && j["MESSAGE_TYPE"] != message::MessageType::REPL) {
    return;
  }

  if (j["MESSAGE_TYPE"] == message::MessageType::REPL) {
    std::cout << "RaftServer(" << state.get_rank() << "): Received REPL message" << std::endl;
    json repl_message = j["REPL"];
    if (repl_message["REPL_TYPE"] == repl::ReplType::CRASH) {
      std::cout << "Crashing " << state.get_rank() << std::endl;
      crashed = true;
    }
  }
  std::cout << "RaftServer(" << state.get_rank() << "): Received message" << std::endl;
  std::cout << j.dump() << std::endl;
}
} // namespace raft