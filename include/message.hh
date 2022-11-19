#pragma once

#include <algorithm>
#include <memory>
#include <iostream>

#include "nlohmann/json.hpp"
#include "command.hh"

using json = nlohmann::json;

class Processus;
class InternProcessus;

/*
** enum of possible message accepted
*/
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
    CLIENT_REQUEST,
    ME_NOT_LEADER,
};

/*
** Association between the previous enum and the json file data
*/
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
        { MessageType::CLIENT_REQUEST, "CLIENT_REQUEST" },
        { MessageType::ME_NOT_LEADER, "ME_NOT_LEADER" },
    })

/*
** Class Message
**
** Specification about a message accepted between servers and clients
*/
class Message
{
public:

    /*
    ** Constructor
    **
    ** @MessageType type : Specify the type of message
    */
    Message(MessageType type)
        : type(type)
        , sender_rank(-1)
        , target_rank(-1){};

    /*
    ** Constructor
    **
    ** @MessageType type : Specify the type of message
    ** @int target_rank : Specify the receiver Id
    ** @int sender_rank : Specify the sender Id
    */
    Message(MessageType type, int target_rank, int sender_rank)
        : type(type)
        , sender_rank(sender_rank)
        , target_rank(target_rank){
        };
    virtual ~Message() = default;

    /*
    ** accept Function
    **
    **  @Processus &process : the reference to the processus that will receive the message
    */
    virtual void accept(Processus &process) = 0;

    /*
    ** serialize Function
    **
    ** Tranform the message into json and dump it
    */
    virtual std::string serialize() const = 0;
    virtual int get_term() {return -42;};

    /*
    ** deserialize Function
    **
    ** Parse a json data to identify a message
    */
    static std::unique_ptr<Message> deserialize(const json &msg);

    MessageType type;
    int sender_rank;
    int target_rank;
};

/*
**  Class ClientRequest
**
**  A specific message of type CLIENT_REQUEST
*/
class ClientRequest : public Message
{
public:
    ClientRequest()
        : Message(MessageType::CLIENT_REQUEST)
        , command(nullptr)
    {};

    ClientRequest(int target_rank, int sender_rank, const std::shared_ptr<Command> &cmd)
        : Message(MessageType::CLIENT_REQUEST, target_rank, sender_rank), command(cmd){};
    
    virtual std::string serialize() const override;
    virtual void accept(Processus &process) override;

    std::shared_ptr<Command> command;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientRequest, type, sender_rank, target_rank, command)

class LogEntry
{
public:
    LogEntry(): term(-1), command(nullptr){};
    LogEntry(int term, const std::shared_ptr<Command> &cmd)
        : term(term), command(cmd){};

    int term;
    std::shared_ptr<Command> command;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LogEntry, term, command)

/*
**  Class ReplCrash
**
**  A specific message of type REPL_CRASH
*/
class ReplCrash : public Message
{
public:
    ReplCrash()
        : Message(MessageType::REPL_CRASH)
    {}
    ReplCrash(int target_rank, int sender_rank)
        : Message(MessageType::REPL_CRASH, target_rank, sender_rank)
    {}

    virtual std::string serialize() const override;
    void accept(Processus &process) override;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplCrash, type, sender_rank, target_rank)

/*
**  Class ReplStart
**
**  A specific message of type REPL_START
*/
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

/*
** Enum of accepted Speed for a ReplSpeed message
*/
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

/*
**  Class ReplSpeed
**
**  A specific message of type ReplSpeed that can take 3 different speed (FAST, MEDIUM, LOW)
*/
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

/*
**  Class RpcMessage
**
**  A specific message of type RPC_MESSAGE
*/
class RpcMessage : public Message
{
public:
    RpcMessage(MessageType type)
        : Message(type){
        };
    RpcMessage(MessageType type, int target_rank, int sender_rank, int term)
        : Message(type, target_rank, sender_rank)
        , term(term)
    {
    }

    void accept(Processus &process) override;
    virtual int get_term() override {return term;};

    int term;
};

/*
**  Class RpcRequestVote
**
**  A specific message of type RPC_REQUEST_VOTE
*/
class RpcRequestVote : public RpcMessage
{
public:
    RpcRequestVote()
        : RpcMessage(MessageType::RPC_REQUEST_VOTE){};
    RpcRequestVote(int target_rank, int sender_rank, int term, 
                   int candidate_uid, int last_log_index, 
                   int last_log_term)
        : RpcMessage(MessageType::RPC_REQUEST_VOTE, target_rank, sender_rank, term)
        , candidate_uid(candidate_uid)
        , last_log_index(last_log_index)
        , last_log_term(last_log_term)
    {
    }

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    int candidate_uid;
    int last_log_index;
    int last_log_term;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcRequestVote, type, sender_rank,
                                   target_rank, term, last_log_index,
                                   last_log_term)

/*
**  Class RpcAppendEntries
**
**  A specific message of type RPC_APPEND_ENTRIES
*/
class RpcAppendEntries : public RpcMessage
{
public:
    RpcAppendEntries()
        : RpcMessage(MessageType::RPC_APPEND_ENTRIES){};
    RpcAppendEntries(int target_rank, int sender_rank, int term, 
                        int leader_uid, int prev_log_index,
                        int prev_log_term, 
                        std::vector<LogEntry> entries,
                        int leader_commit)
        : RpcMessage(MessageType::RPC_APPEND_ENTRIES, target_rank, 
                        sender_rank, term)
        , leader_uid(leader_uid)
        , prev_log_index(prev_log_index)
        , prev_log_term(prev_log_term)
        , entries(entries)
        , leader_commit(leader_commit)
    {std::cout << "RpcAppendEntries creation..." << std::endl;}

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    int leader_uid;
    int prev_log_index;
    int prev_log_term;
    std::vector<LogEntry> entries;
    int leader_commit;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcAppendEntries, type, sender_rank,
                                target_rank, term, leader_uid, prev_log_index,
                                prev_log_term, entries, leader_commit)

/*
**  Class RpcVoteResponse
**
**  A specific message of type RPC_VOTE_RESPONSE
*/
class RpcVoteResponse : public RpcMessage
{
public:
    RpcVoteResponse()
        : RpcMessage(MessageType::RPC_VOTE_RESPONSE){};
    RpcVoteResponse(int target_rank, int sender_rank, int term, 
                    bool vote_granted)
        : RpcMessage(MessageType::RPC_VOTE_RESPONSE, target_rank, sender_rank, term)
        , vote_granted(vote_granted)
    {}

    virtual std::string serialize() const override;
    void accept(Processus &process) override;

    bool vote_granted;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcVoteResponse, type, sender_rank,
                                   target_rank, term, vote_granted)

/*
**  Class RpcAppendEntriesResponse
**
**  A specific message of type RPC_APPEND_ENTRIES_RESPONSE
*/
class RpcAppendEntriesResponse : public RpcMessage
{
public:
    RpcAppendEntriesResponse()
        : RpcMessage(MessageType::RPC_APPEND_ENTRIES_RESPONSE){};
    RpcAppendEntriesResponse(int target_rank, int sender_rank, int term,
                             bool success)
        : RpcMessage(MessageType::RPC_APPEND_ENTRIES_RESPONSE, target_rank,
                sender_rank, term)
        , success(success)
    {}

    virtual std::string serialize() const override;
    void accept(Processus& process) override;

    bool success;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcAppendEntriesResponse, type,
                                sender_rank, target_rank, term, success)

/*
**  Class HandshakeFailure
**
**  A specific message of type HANDSHAKE_FAILURE
*/
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

class MeNotLeader : public Message
{
public:
    MeNotLeader()
        : Message(MessageType::ME_NOT_LEADER){};
    MeNotLeader(int target_rank, int sender_rank, int leader_uid)
        : Message(MessageType::ME_NOT_LEADER, target_rank, sender_rank)
        , leader_uid(leader_uid){};

    virtual std::string serialize() const override;
    void accept(Processus &process) override;
    
    int leader_uid;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MeNotLeader, type, sender_rank,
                                   target_rank, leader_uid);

/*
**  Class HandshakeSuccess
**
**  A specific message of type HANDSHAKE_SUCCESS
*/
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