#include "message.hh"
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

void RpcMessage::accept(Processus &process)
{
    process.receive(*this);
}

void RpcRequestVote::accept(Processus &process)
{
    RpcMessage::accept(process);
    process.receive(*this);
}

void RpcAppendEntries::accept(Processus &process)
{
    RpcMessage::accept(process);
    process.receive(*this);
}

void RpcVoteResponse::accept(Processus &process)
{
    RpcMessage::accept(process);
    process.receive(*this);
}

void RpcAppendEntriesResponse::accept(Processus &process)
{
    RpcMessage::accept(process);
    process.receive(*this);
}

void HandshakeFailure::accept(Processus &process)
{
    process.receive(*this);
}

void MeNotLeader::accept(Processus &process)
{
    process.receive(*this);
}

void HandshakeSuccess::accept(Processus &process)
{
    process.receive(*this);
}

void SuccessLoad::accept(Processus &process)
{
    process.receive(*this);
}

void SuccessList::accept(Processus &process)
{
    process.receive(*this);
}

void ClientRequest::accept(Processus &process)
{
    process.receive(*this);
}

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

std::string MeNotLeader::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string SuccessLoad::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string SuccessList::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string HandshakeSuccess::serialize() const
{
    json j = *this;
    
    return j.dump();
}

std::string ClientRequest::serialize() const
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
    case MessageType::ME_NOT_LEADER:
        return std::make_unique<MeNotLeader>(j.get<MeNotLeader>());
    case MessageType::HANDSHAKE_SUCCESS:
        return std::make_unique<HandshakeSuccess>(j.get<HandshakeSuccess>());
    case MessageType::CLIENT_REQUEST:
        return std::make_unique<ClientRequest>(j.get<ClientRequest>());
    case MessageType::SUCCESS_LOAD:
        return std::make_unique<SuccessLoad>(j.get<SuccessLoad>());
    case MessageType::SUCCESS_LIST:
        return std::make_unique<SuccessList>(j.get<SuccessList>());
    }
    return nullptr;
}