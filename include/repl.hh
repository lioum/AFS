#pragma once

#include <mpi.h>

#include "processus.hh"

class REPL : public Processus
{
public:
    REPL(MPI_Comm com, int nb_servers);

    void work() override;

    virtual void receive(HandshakeFailure &msg) override;
    virtual void receive(HandshakeSuccess &msg) override;

private:
    std::shared_ptr<Message> process_message(const std::string &input);
    bool waiting_for_handshake = false;
};