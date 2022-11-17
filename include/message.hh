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

    MessageType type;
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
    
    //virtual std::string serialize() const override;

    virtual void call_execute(Processus &process) = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientMessage, type, sender_rank, target_rank)

class LogEntry
{
public:
    LogEntry() = default;
    LogEntry(int term, std::shared_ptr<ClientMessage> command)
        : term(term)
        , command(command)
    {}

        virtual void accept(std::shared_ptr<Processus> process) = 0;

        const MessageType type;
        const unsigned int sender_rank;
        const unsigned int target_rank;
    };

    class LogEntry
    {
    public:
        LogEntry(unsigned int term, std::string command, unsigned int client_id)
            : term(term), command(command), client_id(client_id)
        {}
    
        unsigned int term;
        std::string command;
        unsigned int client_id;
        //unsigned int command_uid;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LogEntry, term, command, client_id)

    class ReplCrash : public Message
    {
    public:
        ReplCrash()
            : Message(MessageType::REPL_CRASH)
        {}
        ReplCrash(unsigned int target_rank, unsigned int sender_rank)
            : Message(MessageType::REPL_CRASH, target_rank, sender_rank)
        {}

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

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplSpeed, type, sender_rank,
                                       target_rank, speed)
    
    class RpcMessage : public Message
    {
    public:
        RpcMessage(MessageType type)
            : Message(type){};
        RpcMessage(MessageType type, unsigned int target_rank, unsigned int sender_rank, unsigned int term)
            : Message(type, target_rank, sender_rank)
            , term(term)
        {}

        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

        unsigned int term;
    };

    class RpcRequestVote : public RpcMessage
    {
    public:
        RpcRequestVote()
            : RpcMessage(MessageType::RPC_REQUEST_VOTE){};
        RpcRequestVote(unsigned int target_rank, unsigned int sender_rank, unsigned int term, unsigned int candidate_uid,
                       int last_log_index, int last_log_term)
            : RpcMessage(MessageType::RPC_REQUEST_VOTE, target_rank, sender_rank, term)
            , candidate_uid(candidate_uid)
            , last_log_index(last_log_index)
            , last_log_term(last_log_term)
        {}

        void accept(std::shared_ptr<Processus> process) override
        {
            RpcMessage::accept(process);
            process->receive(this);
        }

        unsigned int candidate_uid;
        unsigned int last_log_index;
        unsigned int last_log_term;
    };

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcRequestVote, type, sender_rank,
                                   target_rank, term, last_log_index,
                                   last_log_term)

    class RpcAppendEntries : public RpcMessage
    {
    public:
        RpcAppendEntries()
            : RpcMessage(MessageType::RPC_APPEND_ENTRIES){};
        RpcAppendEntries(unsigned int target_rank, unsigned int sender_rank, unsigned int term, unsigned int leader_uid,
                         unsigned int prev_log_index, unsigned int prev_log_term,
                         std::vector<LogEntry> entries, unsigned int leader_commit)
            : RpcMessage(MessageType::RPC_APPEND_ENTRIES, target_rank, sender_rank, term)
            , leader_uid(leader_uid)
            , prev_log_index(prev_log_index)
            , prev_log_term(prev_log_term)
            , entries(entries)
            , leader_commit(leader_commit)
        {}
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

        unsigned int leader_uid;
        unsigned int prev_log_index;
        unsigned prev_log_term;
        std::vector<LogEntry> entries;
        unsigned leader_commit;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcAppendEntries, type, sender_rank,
                                       target_rank, term, leader_uid, prev_log_index,
                                       prev_log_term, entries, leader_commit)

    class RpcVoteResponse : public RpcMessage
    {
    public:
        RpcVoteResponse()
            : RpcMessage(MessageType::RPC_VOTE_RESPONSE){};
        RpcVoteResponse(unsigned int target_rank, unsigned int sender_rank, unsigned int term,
                        bool vote_granted)
            : RpcMessage(MessageType::RPC_VOTE_RESPONSE, target_rank, sender_rank, term)
            , vote_granted(vote_granted)
        {}
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

        bool vote_granted;
    };

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcVoteResponse, type, sender_rank,
                                   target_rank, term, vote_granted)

    class RpcAppendEntriesResponse : public RpcMessage
    {
    public:
        RpcAppendEntriesResponse()
            : RpcMessage(MessageType::RPC_APPEND_ENTRIES_RESPONSE){};
        RpcAppendEntriesResponse(unsigned int target_rank, unsigned int sender_rank, unsigned int term,
                                 bool success)
            : RpcMessage(MessageType::RPC_APPEND_ENTRIES_RESPONSE, target_rank,
                      sender_rank, term)
            , success(success)
        {}
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }
        bool success;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcAppendEntriesResponse, type,
                                       sender_rank, target_rank, term, success)

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
    ClientAppend(int target_rank, int sender_rank, int uid,
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
