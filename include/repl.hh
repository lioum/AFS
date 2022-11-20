#pragma once

#include <mpi.h>

#include "processus.hh"

/*
** Class REPL 
**
** Instance of a extern Processus that will send order to others Processus
*/
class REPL : public Processus
{
public:
    /*
    ** Constrcutor
    **
    ** @MPI_Comm com : the communicator provided by MPI that contains all MPI processes
    ** @int nb_servers : the number of servers
    */
    REPL(MPI_Comm com, int nb_servers, int nb_clients);

    /*
    ** work Function
    **
    ** The Processus will send to a server his order and wait for an answer
    */
    void work() override;

    /*
    ** receive Function
    **
    ** case HandshakeFailure : receive a message of failure and doesnt keep it
    ** case HandshakeSuccess : receive a message of success and keep it
    */
    virtual void receive(HandshakeFailure &msg) override;
    virtual void receive(HandshakeSuccess &msg) override;

private:
    /*
    ** process_message Function
    **
    ** Create the order to be sent between CRASH, SPEED, START
    */
    std::shared_ptr<Message> process_message(const std::string &input);
    bool waiting_for_handshake = false;
    int nb_clients;
};