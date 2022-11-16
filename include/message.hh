#pragma once

#include <memory>

#include "nlohmann/json.hpp"

class LogEntry;

using json = nlohmann::json;

class Processus;

enum class MessageType
{
    REPL_CRASH = 0,
    REPL_SPEED,
    REPL_START,
    RPC_REQUEST_VOTE,
    RPC_APPEND_ENTRIES,
    RPC_VOTE_RESPONSE,
    RPC_APPEND_ENTRIES_RESPONSE,
    HANDSHAKE_FAILURE,
    HANDSHAKE_SUCCESS,
    CLIENT_LOAD,
    CLIENT_LIST,
    CLIENT_APPEND,
    CLIENT_DELETE,
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    MessageType,
    {
        { MessageType::REPL_CRASH, "REPL_CRASH" },
        { MessageType::REPL_SPEED, "REPL_SPEED" },
        { MessageType::REPL_START, "REPL_START" },
        { MessageType::RPC_REQUEST_VOTE, "RPC_REQUEST_VOTE" },
        { MessageType::RPC_APPEND_ENTRIES, "RPC_APPEND_ENTRIES" },
        { MessageType::RPC_VOTE_RESPONSE, "RPC_VOTE_RESPONSE" },
        { MessageType::RPC_APPEND_ENTRIES_RESPONSE,
          "RPC_APPEND_ENTRIES_RESPONSE" },
        { MessageType::HANDSHAKE_FAILURE, "HANDSHAKE_FAILURE" },
        { MessageType::HANDSHAKE_SUCCESS, "HANDSHAKE_SUCCESS" },
        { MessageType::CLIENT_LOAD, "CLIENT_LOAD" },
        { MessageType::CLIENT_LIST, "CLIENT_LIST" },
        { MessageType::CLIENT_APPEND, "CLIENT_APPEND" },
        { MessageType::CLIENT_DELETE, "CLIENT_DELETE" },
    })

class Message
{
public:
    Message(MessageType type)
        : type(type)
        , sender_rank(-1)
        , target_rank(-1){};
    Message(MessageType type, int target_rank, int sender_rank)
        : type(type)
        , sender_rank(sender_rank)
        , target_rank(target_rank){};
    virtual ~Message() = default;

    virtual void accept(Processus &process) = 0;
    virtual std::string serialize() const = 0;

    static std::unique_ptr<Message> deserialize(const json &msg);

    const MessageType type;
    int sender_rank;
    int target_rank;
};

class ClientMessage : public Message
{
public:
    ClientMessage(MessageType type)
        : Message(type){};
    ClientMessage(MessageType type, int target_rank, int sender_rank)
        : Message(type, target_rank, sender_rank){};
    
    virtual std::string serialize() const override;

    virtual void call_execute(Processus &process) = 0;
};

class LogEntry
{
public:
    LogEntry() = default;
    LogEntry(int term, std::shared_ptr<ClientMessage> command)
        : term(term)
        , command(command)
    {}

    int term;
    std::shared_ptr<ClientMessage> command;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LogEntry, term, command)

class ReplCrash : public Message
{
public:
    ReplCrash()
        : Message(MessageType::REPL_CRASH){};
    ReplCrash(int target_rank, int sender_rank)
        : Message(MessageType::REPL_CRASH, target_rank, sender_rank){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplCrash, type, sender_rank, target_rank)

class ReplStart : public Message
{
public:
    ReplStart()
        : Message(MessageType::REPL_START){};
    ReplStart(int target_rank, int sender_rank)
        : Message(MessageType::REPL_START, target_rank, sender_rank){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplStart, type, sender_rank, target_rank)

enum class Speed
{
    FAST = 0,
    MEDIUM,
    LOW,
};

NLOHMANN_JSON_SERIALIZE_ENUM(Speed,
                             {
                                 { Speed::FAST, "FAST" },
                                 { Speed::MEDIUM, "MEDIUM" },
                                 { Speed::LOW, "LOW" },
                             })

class ReplSpeed : public Message
{
public:
    ReplSpeed()
        : Message(MessageType::REPL_SPEED){};
    ReplSpeed(int target_rank, int sender_rank, Speed speed)
        : Message(MessageType::REPL_SPEED, target_rank, sender_rank)
        , speed(speed){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    Speed speed = Speed::MEDIUM;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplSpeed, type, sender_rank, target_rank,
                                   speed)

class RpcRequestVote : public Message
{
public:
    RpcRequestVote()
        : Message(MessageType::RPC_REQUEST_VOTE){};
    RpcRequestVote(int target_rank, int sender_rank, int term,
                   int last_log_index, int last_log_term)
        : Message(MessageType::RPC_REQUEST_VOTE, target_rank, sender_rank)
        , term(term)
        , last_log_index(last_log_index)
        , last_log_term(last_log_term){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    int term;
    int last_log_index;
    int last_log_term;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcRequestVote, type, sender_rank,
                                   target_rank, term, last_log_index,
                                   last_log_term)

class RpcAppendEntries : public Message
{
public:
    RpcAppendEntries()
        : Message(MessageType::RPC_APPEND_ENTRIES){};
    RpcAppendEntries(int target_rank, int sender_rank, int term,
                     int prev_log_index, int prev_log_term,
                     std::vector<LogEntry> entries, int leader_commit)
        : Message(MessageType::RPC_APPEND_ENTRIES, target_rank, sender_rank)
        , term(term)
        , prev_log_index(prev_log_index)
        , prev_log_term(prev_log_term)
        , entries(entries)
        , leader_commit(leader_commit){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    int term;
    int prev_log_index;
    int prev_log_term;
    std::vector<LogEntry> entries;
    int leader_commit;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcAppendEntries, type, sender_rank,
                                   target_rank, term, prev_log_index,
                                   prev_log_term, entries, leader_commit)

class RpcVoteResponse : public Message
{
public:
    RpcVoteResponse()
        : Message(MessageType::RPC_VOTE_RESPONSE){};
    RpcVoteResponse(int target_rank, int sender_rank, int term,
                    bool vote_granted)
        : Message(MessageType::RPC_VOTE_RESPONSE, target_rank, sender_rank)
        , term(term)
        , vote_granted(vote_granted){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    int term;
    bool vote_granted;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcVoteResponse, type, sender_rank,
                                   target_rank, term, vote_granted)

class RpcAppendEntriesResponse : public Message
{
public:
    RpcAppendEntriesResponse()
        : Message(MessageType::RPC_APPEND_ENTRIES_RESPONSE){};
    RpcAppendEntriesResponse(int target_rank, int sender_rank, int term,
                             bool success, int last_log_index)
        : Message(MessageType::RPC_APPEND_ENTRIES_RESPONSE, target_rank,
                  sender_rank)
        , term(term)
        , success(success)
        , last_log_index(last_log_index){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    int term;
    bool success;
    int last_log_index;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcAppendEntriesResponse, type, sender_rank,
                                   target_rank, term, success, last_log_index)

class HandshakeFailure : public Message
{
public:
    HandshakeFailure()
        : Message(MessageType::HANDSHAKE_FAILURE){};
    HandshakeFailure(int target_rank, int sender_rank)
        : Message(MessageType::HANDSHAKE_FAILURE, target_rank, sender_rank)
        , data(json()){};
    HandshakeFailure(int target_rank, int sender_rank, const json &data)
        : Message(MessageType::HANDSHAKE_FAILURE, target_rank, sender_rank)
        , data(data){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    json data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HandshakeFailure, type, sender_rank,
                                   target_rank, data)

class HandshakeSuccess : public Message
{
public:
    HandshakeSuccess()
        : Message(MessageType::HANDSHAKE_SUCCESS){};
    HandshakeSuccess(int target_rank, int sender_rank)
        : Message(MessageType::HANDSHAKE_SUCCESS, target_rank, sender_rank)
        , data(json()){};
    HandshakeSuccess(int target_rank, int sender_rank, const json &data)
        : Message(MessageType::HANDSHAKE_SUCCESS, target_rank, sender_rank)
        , data(data){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    json data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HandshakeSuccess, type, sender_rank,
                                   target_rank, data)

class ClientLoad : public ClientMessage
{
public:
    ClientLoad()
        : ClientMessage(MessageType::CLIENT_LOAD){};
    ClientLoad(int target_rank, int sender_rank, const std::string &filename,
               const std::string &content)
        : ClientMessage(MessageType::CLIENT_LOAD, target_rank, sender_rank)
        , filename(filename)
        , content(content){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;
    void call_execute(Processus &process) override;

    std::string filename;
    std::string content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientLoad, type, sender_rank, target_rank,
                                   filename, content)

class ClientList : public ClientMessage
{
public:
    ClientList()
        : ClientMessage(MessageType::CLIENT_LIST){};
    ClientList(int target_rank, int sender_rank)
        : ClientMessage(MessageType::CLIENT_LIST, target_rank, sender_rank){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;
    void call_execute(Processus &process) override;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientList, type, sender_rank, target_rank)

class ClientAppend : public ClientMessage
{
public:
    ClientAppend()
        : ClientMessage(MessageType::CLIENT_APPEND){};
    ClientAppend(int target_rank, int sender_rank, const std::string &filename,
                 const std::string &content)
        : ClientMessage(MessageType::CLIENT_APPEND, target_rank, sender_rank)
        , content(content)
        , uid(uid){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;
    void call_execute(Processus &process) override;

    std::string content;
    int uid;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientAppend, type, sender_rank, target_rank,
                                   content, uid)

class ClientDelete : public ClientMessage
{
public:
    ClientDelete()
        : ClientMessage(MessageType::CLIENT_DELETE){};
    ClientDelete(int target_rank, int sender_rank, int uid)
        : ClientMessage(MessageType::CLIENT_DELETE, target_rank, sender_rank)
        , uid(uid){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;
    void call_execute(Processus &process) override;

    int uid;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientDelete, type, sender_rank, target_rank,
                                   uid)
