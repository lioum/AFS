#pragma once

#include <queue>

#include "processus.hh"
#include "repl_message.hh"

namespace client
{
    class Client : public Processus
    {
    public:
        Client(MPI_Comm com, int nb_servers);

        void on_message_callback(std::shared_ptr<message::Message> message) override;

        void work() override;

    private:
        std::queue<HandshakeSuccess> handshake_queue;
        std::map<int, std::string> uids;

        int target_rank;
        bool target_alive = false;

        void receive(HandshakeFailure &msg) override;
        void receive(HandshakeSuccess &msg) override;
    };

} // namespace client