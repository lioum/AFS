#include "raft_server.hh"

namespace raft {
RaftServer::RaftServer(MPI_Comm com) : Server(com) {}

void RaftServer::on_message_callback(std::shared_ptr<message::Message> message) {
  std::cout << "RaftServer: Received message" << std::endl;
}
} // namespace raft