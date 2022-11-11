#pragma once
#include "repl_message.hh"
#include "server.hh"

#include <mpi.h>

namespace repl
{
    class REPL : public Server
    {
    public:
        REPL(MPI_Comm com);
        //~REPL();

        void run();

    private:
        std::optional<REPL_message> process_message(std::string input);
        int rank;
        int size;
    };
}