#pragma once
#include "repl_message.hh"
#include "server.hh"

#include <mpi.h>

namespace repl
{
    class REPL : public Server
    {
    public:
        REPL(MPI_Comm com, int nb_servers);

        void work();

        void on_message_callback(std::shared_ptr<message::Message> message) override;

    private:
        std::shared_ptr<REPL_message> process_message(std::string input);
        bool running;
    };
}