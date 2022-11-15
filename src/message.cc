#include "message.hh"

void ReplCrash::accept(Processus &process)
{
    process.receive(*this);
}

void ReplStart::accept(Processus &process)
{
    process.receive(*this);
}

void ReplSpeed::accept(Processus &process)
{
    process.receive(*this);
}

void RpcRequestVote::accept(Processus &process)
{
    process.receive(*this);
}

void RpcAppendEntries::accept(Processus &process)
{
    process.receive(*this);
}

void RpcVoteResponse::accept(Processus &process)
{
    process.receive(*this);
}

void RpcAppendEntriesResponse::accept(Processus &process)
{
    process.receive(*this);
}

void HandshakeFailure::accept(Processus &process)
{
    process.receive(*this);
}

void HandshakeSuccess::accept(Processus &process)
{
    process.receive(*this);
}

void ClientLoad::accept(Processus &process)
{
    process.receive(*this);
}

void ClientLoad::call_execute(Processus &process) 
{
    process.call_execute(*this);
}

void ClientList::accept(Processus &process)
{
    process.receive(*this);
}

void ClientList::call_execute(Processus &process) 
{
    process.call_execute(*this);
}

void ClientAppend::accept(Processus &process)
{
    process.receive(*this);
}

void ClientAppend::call_execute(Processus &process) 
{
    process.call_execute(*this);
}

void ClientDelete::accept(Processus &process)
{
    process.receive(*this);
}

void ClientDelete::call_execute(Processus &process) 
{
    process.call_execute(*this);
}