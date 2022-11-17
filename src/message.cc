#include "processus.hh"


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
    process.execute(*this);
}

void ClientList::accept(Processus &process)
{
    process.receive(*this);
}

void ClientList::call_execute(Processus &process) 
{
    process.execute(*this);
}

void ClientAppend::accept(Processus &process)
{
    process.receive(*this);
}

void ClientAppend::call_execute(Processus &process) 
{
    process.execute(*this);
}

void ClientDelete::accept(Processus &process)
{
    process.receive(*this);
}

void ClientDelete::call_execute(Processus &process) 
{
    process.execute(*this);
}

/*std::string ClientMessage::serialize() const
{
    json j = *this;

    return j.dump();
}*/

std::string ReplCrash::serialize() const
{
    json j = *this;
    
    return j.dump();
} 
 
std::string ReplStart::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string ReplSpeed::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string RpcRequestVote::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string RpcAppendEntries::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string RpcVoteResponse::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string RpcAppendEntriesResponse::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string HandshakeFailure::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string HandshakeSuccess::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string ClientLoad::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string ClientList::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string ClientAppend::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string ClientDelete::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::unique_ptr<Message> Message::deserialize(const json &j)
{
    MessageType msg_type = j["type"].get<MessageType>();
    switch (msg_type)
    {
    case MessageType::REPL_CRASH:
        return std::make_unique<ReplCrash>(j.get<ReplCrash>());
    case MessageType::REPL_SPEED:
        return std::make_unique<ReplSpeed>(j.get<ReplSpeed>());
    case MessageType::REPL_START:
        return std::make_unique<ReplStart>(j.get<ReplStart>());
    case MessageType::RPC_REQUEST_VOTE:
        return std::make_unique<RpcRequestVote>(j.get<RpcRequestVote>());
    case MessageType::RPC_APPEND_ENTRIES:
        return std::make_unique<RpcAppendEntries>(j.get<RpcAppendEntries>());
    case MessageType::RPC_VOTE_RESPONSE:
        return std::make_unique<RpcVoteResponse>(j.get<RpcVoteResponse>());
    case MessageType::RPC_APPEND_ENTRIES_RESPONSE:
        return std::make_unique<RpcAppendEntriesResponse>(
            j.get<RpcAppendEntriesResponse>());
    case MessageType::HANDSHAKE_FAILURE:
        return std::make_unique<HandshakeFailure>(j.get<HandshakeFailure>());
    case MessageType::HANDSHAKE_SUCCESS:
        return std::make_unique<HandshakeSuccess>(j.get<HandshakeSuccess>());
    case MessageType::CLIENT_LOAD:
        return std::make_unique<ClientLoad>(j.get<ClientLoad>());
    case MessageType::CLIENT_LIST:
        return std::make_unique<ClientList>(j.get<ClientList>());
    case MessageType::CLIENT_APPEND:
        return std::make_unique<ClientAppend>(j.get<ClientAppend>());
    case MessageType::CLIENT_DELETE:
        return std::make_unique<ClientDelete>(j.get<ClientDelete>());
    }
    return nullptr;
}