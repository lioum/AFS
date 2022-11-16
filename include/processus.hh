#pragma once

#include <chrono>
#include <cstddef>
#include <deque>
#include <filesystem>
#include <mpi.h>
#include <optional>

#include "message.hh"

class Processus
{
public:
    Processus(MPI_Comm com, int nb_servers);

    std::shared_ptr<Message> listen();
    void send(const Message &message);
    void broadcast_to_servers(Message &message);

    void run();
    virtual void work() = 0;

    virtual void receive(RpcRequestVote &msg){ msg = msg;};
    virtual void receive(RpcAppendEntries &msg){ msg = msg;};
    virtual void receive(RpcVoteResponse &msg){ msg = msg;};
    virtual void receive(RpcAppendEntriesResponse &msg){ msg = msg;};
    virtual void receive(HandshakeFailure &msg){ msg = msg;};
    virtual void receive(HandshakeSuccess &msg){ msg = msg;};
    virtual void receive(ClientLoad &msg){ msg = msg;};
    virtual void receive(ClientList &msg){ msg = msg;};
    virtual void receive(ClientAppend &msg){ msg = msg;};
    virtual void receive(ClientDelete &msg){ msg = msg;};
    virtual void receive(ReplCrash &msg){ msg = msg;};
    virtual void receive(ReplSpeed &msg){ msg = msg;};
    virtual void receive(ReplStart &msg){ msg = msg;};

    virtual void execute(ClientLoad &msg){ msg = msg;};
    virtual void execute(ClientList &msg){ msg = msg;};
    virtual void execute(ClientAppend &msg){ msg = msg;};
    virtual void execute(ClientDelete &msg){ msg = msg;};

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

    virtual void receive(ReplCrash &msg) override;
    virtual void receive(ReplSpeed &msg) override;
    virtual void receive(ReplStart &msg) override;

protected:
    bool crashed;
    bool started;
    Speed speed;

    std::filesystem::path working_folder_path;
};