#pragma once

#include <queue>

#include "server.hh"
#include "repl_message.hh"

namespace raft
{
class RaftServer : public Server {
    public:
      RaftServer(MPI_Comm com, int nb_servers);
      
      void on_message_callback(std::shared_ptr<message::Message> message) override;
      
      void work() override;
    private:
        bool crashed;
        bool started;
        repl::ReplSpeed speed;
        std::map<int, std::string> uids;
        
        std::queue<std::shared_ptr<message::Message>> message_queue;
        
        void on_receive_repl(std::shared_ptr<message::Message> message);

        void on_receive_rpc(std::shared_ptr<message::Message> message);

        void process_message_client(std::shared_ptr<message::Message> message);
        
        void broadcast_to_servers(std::shared_ptr<message::Message> message);
  };

} // namespace raft