#pragma once

#include "nlohmann/json.hpp"

using json = nlohmann::json;

//enum class Message: int {
//    ELECTION = 0,
//};
//


namespace message
{
    enum class MessageType
    {
        REPL,
        RPC,
    };

    class Message
    {
    public:
        explicit Message(MessageType type);
        virtual ~Message() = default;

        std::string serialize() const;
        static std::shared_ptr<Message> deserialize(const std::string &message);

    private:
        virtual const std::string serialize_json() = 0;

        const MessageType type;
    };
}


