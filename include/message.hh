#pragma once

#include "nlohmann/json.hpp"

using json = nlohmann::json;

//enum class Message: int {
//    ELECTION = 0,
//};
//


namespace message
{
    enum MessageType
    {
        REPL = 0,
        RPC,
        HANDSHAKE,
    };

    class Message
    {
    public:
        explicit Message(MessageType type, int sender_rank, int target_rank);
        virtual ~Message() = default;
        int get_target_rank() const { return target_rank; }

        std::string serialize() const;
        static std::shared_ptr<Message> deserialize(const std::string &message);
        virtual json serialize_json() const = 0;

    protected:
        const MessageType type;
        const int sender_rank;
        const int target_rank;
    };
}


