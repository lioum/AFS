#pragma once

#include "server.hh"
#include "repl_message.hh"

namespace raft
{
class RaftServer : public Server {
    public:
      RaftServer(MPI_Comm com, int nb_servers);
      
      void on_message_callback(std::shared_ptr<message::Message> message) override;
      
      void work();
    private:
        bool crashed;
        bool started;
        repl::ReplSpeed speed;
        
        void on_receive_repl(json j);

        void on_receive_rpc(json j);

        void on_receive_client(json j);
        
        void broadcast_to_servers(std::shared_ptr<message::Message> message);
  };

} // namespace raft