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
    };

    class Message
    {
    public:
        explicit Message(MessageType type);
        virtual ~Message() = default;

        std::string serialize() const;
        static std::shared_ptr<Message> deserialize(const std::string &message);
        virtual json serialize_json() const = 0;

    private:
        const MessageType type;
    };
}


