#pragma once

#include <chrono>
#include <cstddef>
#include <deque>
#include <filesystem>
#include <mpi.h>
#include <optional>

#include "message.hh"

using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

/*
** Class Processus
**
** Instance of a proccessus that supervise servers and clients when they want to interact with each other
*/
class Processus
{
public:
    /*
    ** Constructor
    **
    ** @MPI_Comm com : the communicator provided by MPI that contains all MPI processes
    ** @int nb_servers : the number of servers
    */
    Processus(MPI_Comm com, int nb_servers);

    /*
    ** listen Function
    **
    ** Listening for a message from a json source
    */
    std::shared_ptr<Message> listen();

    /*
    ** send Function
    **
    ** @Message &message : reference to the message to be sent
    ** Send the message once serialized to his target
    */
    void send(const Message &message);

    /*
    ** broadcast_to_servers Function
    **
    ** @Message & message : reference to the message to be broadcasted
    ** Send the message to all the servers
    */
    void broadcast_to_servers(Message &message);

    /*
    ** run Function
    **
    ** The processus will start to listen and eventually send the listened message
    */
    void run();

    /*
    ** work Abstract Function
    **
    ** the objectiv is to send the message to clients or servers 
    */
    virtual void work() = 0;

    /*
    ** receive Abstract Function
    **
    ** the objectiv is to receive a message of a specific type
    */
    virtual void receive(RpcRequestVote &){};
    virtual void receive(RpcAppendEntries &){};
    virtual void receive(RpcVoteResponse &){};
    virtual void receive(RpcAppendEntriesResponse &){};
    virtual void receive(HandshakeFailure &){};
    virtual void receive(HandshakeSuccess &){};
    virtual void receive(ClientRequest &){};
    virtual void receive(ReplCrash &){};
    virtual void receive(ReplSpeed &){};
    virtual void receive(ReplStart &){};
    virtual void receive(ReplRecovery &){};
    virtual void receive(RpcMessage &){};
    virtual void receive(MeNotLeader &){};
    virtual void receive(SuccessLoad &){};
    virtual void receive(SuccessList &){};

    MPI_Comm com;

protected:
    int uid; // MPI rank
    int nb_servers; // MPI size
};

/*
** InternProcessus
**
** A more precise processus that will handle command and their execution
*/
class InternProcessus : public Processus
{
public:
    InternProcessus(MPI_Comm com, int nb_servers,
                    const std::filesystem::path &root_folder_path);

    /*
    ** receive Abstract Function
    **
    ** the objectiv is to receive a message of a specific type
    */
    virtual void receive(RpcMessage &msg) override;
    virtual void receive(ReplCrash &msg) override;
    virtual void receive(ReplSpeed &msg) override;
    virtual void receive(ReplStart &msg) override;
    virtual void receive(ReplRecovery &msg) override;

    /*
    ** execute Abstract Function
    **
    ** the objectiv is to execute a command of a specific type
    */
    virtual void execute(Command &){};
    virtual void execute(Delete &){};
    virtual void execute(Load &){};
    virtual void execute(Append &){};
    virtual void execute(List &){};
    virtual void execute(ClientDelete &){};

protected:
    bool crashed;
    bool started;
    Speed speed;
    nanoseconds sleeping_time;

    std::filesystem::path working_folder_path;
};