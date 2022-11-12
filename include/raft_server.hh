#pragma once

#include "server.hh"

namespace raft
{
class RaftServer : public Server {
    public:
      RaftServer(MPI_Comm com);
      void on_message_callback(std::shared_ptr<message::Message> message) override;
  };
} // namespace raft