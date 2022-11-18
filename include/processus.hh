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

class Processus
{
public:
    Processus(MPI_Comm com, int nb_servers);

    std::shared_ptr<Message> listen();
    void send(const Message &message);
    void broadcast_to_servers(Message &message);

    void run();
    virtual void work() = 0;

    virtual void receive(RpcRequestVote &){};
    virtual void receive(RpcAppendEntries &){};
    virtual void receive(RpcVoteResponse &){};
    virtual void receive(RpcAppendEntriesResponse &){};
    virtual void receive(HandshakeFailure &){};
    virtual void receive(HandshakeSuccess &){};
    virtual void receive(ClientRequest &){};
    virtual void receive(ClientLoad &){};
    virtual void receive(ClientList &){};
    virtual void receive(ClientAppend &){};
    virtual void receive(ClientDelete &){};
    virtual void receive(ReplCrash &){};
    virtual void receive(ReplSpeed &){};
    virtual void receive(ReplStart &){};
    virtual void receive(RpcMessage &) {};

    MPI_Comm com;

protected:
    int uid; // MPI rank
    int nb_servers; // MPI size
};

class InternProcessus : public Processus
{
public:
    InternProcessus(MPI_Comm com, int nb_servers,
                    const std::filesystem::path &root_folder_path);

    virtual void receive(RpcMessage &msg) override;
    virtual void receive(ReplCrash &msg) override;
    virtual void receive(ReplSpeed &msg) override;
    virtual void receive(ReplStart &msg) override;

    virtual void execute(ClientLoad &){};
    virtual void execute(ClientList &){};
    virtual void execute(ClientAppend &){};
    virtual void execute(ClientDelete &){};

protected:
    bool crashed;
    bool started;
    Speed speed;
    nanoseconds sleeping_time;

    std::filesystem::path working_folder_path;
};