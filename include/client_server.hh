#pragma once

#include <queue>

#include "server.hh"
#include "repl_message.hh"

namespace client
{
class ClientServer : public Server {
    public:
      ClientServer(MPI_Comm com, int nb_servers);
      
      void on_message_callback(std::shared_ptr<message::Message> message) override;
      
      void work() override;
    private:
        bool crashed;
        bool started;
        repl::ReplSpeed speed;
        std::map<int, std::string> uids;
        
        int target_rank;
        bool target_alive = false;
        
        std::queue<std::shared_ptr<message::Message>> message_queue;
        
        void on_receive_repl(std::shared_ptr<message::Message> message);
        void on_receive_rpc(std::shared_ptr<message::Message> message);
  };

} // namespace client