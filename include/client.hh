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

        void work() override;

    private:
        int target_rank;
        bool target_alive = false;

        void receive(HandshakeFailure &msg) override;
        void receive(HandshakeSuccess &msg) override;
    };

} // namespace client