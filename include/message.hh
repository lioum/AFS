#pragma once

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace message
{
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
            , target_rank(-1)
            , sender_rank(-1){};
        Message(MessageType type, int target_rank, int sender_rank)
            : type(type)
            , target_rank(target_rank)
            , sender_rank(sender_rank){};
        virtual ~Message() = default;

        int get_target_rank() const
        {
            return target_rank;
        }
        int get_sender_rank() const
        {
            return sender_rank;
        }
        MessageType get_type() const
        {
            return type;
        }

        virtual void accept(std::shared_ptr<Processus> process) = 0;

        const MessageType type;
        const int sender_rank;
        const int target_rank;
    };

    class ReplCrash : public Message
    {
    public:
        ReplCrash()
            : Message(MessageType::REPL_CRASH)
        {}
        ReplCrash(int target_rank, int sender_rank)
            : Message(MessageType::REPL_CRASH, target_rank, sender_rank)
        {}

        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplCrash, type, sender_rank,
                                       target_rank)

    class ReplStart : public Message
    {
    public:
        ReplStart()
            : Message(MessageType::REPL_START){};
        ReplStart(int target_rank, int sender_rank)
            : Message(MessageType::REPL_START, target_rank, sender_rank)
        {}
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplStart, type, sender_rank,
                                       target_rank)

    class ReplSpeed : public Message
    {
    public:
        ReplSpeed()
            : Message(MessageType::REPL_SPEED){};
        ReplSpeed(int target_rank, int sender_rank, int speed)
            : Message(MessageType::REPL_SPEED, target_rank, sender_rank)
            , speed(speed)
        {}
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

        int speed = -1;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReplSpeed, type, sender_rank,
                                       target_rank, speed)

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
            , last_log_term(last_log_term)
        {}

        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        };

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
            , leader_commit(leader_commit)
        {}
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

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
            , vote_granted(vote_granted)
        {}
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

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
            , last_log_index(last_log_index)
        {}
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }
        int term;
        bool success;
        int last_log_index;
    };

    NLHOMANN_DEFINE_TYPE_NON_INTRUSIVE(RpcAppendEntriesResponse, type,
                                       sender_rank, target_rank, term, success,
                                       last_log_index)

    class HandshakeFailure : public Message
    {
    public:
        HandshakeFailure()
            : Message(MessageType::HANDSHAKE_FAILURE){};

        HandshakeFailure(int target_rank, int sender_rank, const json &data)
            : Message(MessageType::HANDSHAKE_FAILURE, target_rank, sender_rank)
            , data(data){};
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

        json data;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HandshakeFailure, type, sender_rank,
                                       target_rank, data)

    class HandshakeSuccess : public Message
    {
    public:
        HandshakeSuccess()
            : Message(MessageType::HANDSHAKE_SUCCESS){};
        HandshakeFailure(int target_rank, int sender_rank, const json &data)
            : Message(MessageType::HANDSHAKE_SUCCESS, target_rank, sender_rank)
            , data(data){};
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

        json data;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HandshakeSuccess, type, sender_rank,
                                       target_rank, data)

    class ClientLoad : public Message
    {
    public:
        ClientLoad()
            : Message(MessageType::CLIENT_LOAD){};
        ClientLoad(int target_rank, int sender_rank, const json &data)
            : Message(MessageType::CLIENT_LOAD, target_rank, sender_rank)
            , data(data){};
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

        std::string filename;
        std::string content;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientLoad, type, sender_rank,
                                       target_rank, filename, content)

    class ClientList : public Message
    {
    public:
        ClientList()
            : Message(MessageType::CLIENT_LIST){};
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }
    };

    NLOHMAN_DEFINE_TYPE_NON_INTRUSIVE(ClientList, type, sender_rank,
                                      target_rank)

    class ClientAppend : public Message
    {
    public:
        ClientAppend()
            : Message(MessageType::CLIENT_APPEND){};
        ClientAppend(int target_rank, int sender_rank,
                     const std::string &filename, const string &content)
            : Message(MessageType::CLIENT_APPEND, target_rank, sender_rank)
            , content(content)
            , uid(uid){};
        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

    private:
        std::string content;
        int uid;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientAppend, type, sender_rank,
                                       target_rank, content, uid)

    class ClientDelete : public Message
    {
    public:
        ClientDelete()
            : Message(MessageType::CLIENT_DELETE)
        {}
        ClientDelete(int target_rank, int sender_rank, int uid)
            : Message(MessageType::CLIENT_DELETE, int target_rank,
                      int sender_rank)
            , uid(uid){};

        void accept(std::shared_ptr<Processus> process) override
        {
            process->receive(this);
        }

    private:
        int uid;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ClientDelete, type, sender_rank,
                                       target_rank, uid)

} // namespace message
