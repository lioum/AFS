#pragma once
#include "repl_message.hh"
#include "server.hh"

namespace repl
{
    class REPL : public Server
    {
    public:
        REPL();
        ~REPL();

        void run();

    private:
        std::optional<REPL_message> process_message(std::string input);
    };
}