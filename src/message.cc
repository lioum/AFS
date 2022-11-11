#include "message.hh"
#include <memory>

#include "repl_message.hh"

namespace message
{
    Message::Message(MessageType type)
        : type(type)
    {}

    std::string Message::serialize() const
    {
        json json;
        switch (type)
        {
        case MessageType::REPL:
            json["REPL"] = this->serialize();
            break;
        case MessageType::RPC:
            json["RPC"] = this->serialize();
            break;
        }
        json["MESSAGE_TYPE"] = this->type;
        return json.dump();
    }

    std::shared_ptr<Message> Message::deserialize(const std::string &message)
    {
        auto json = json::parse(message);
        auto type = json["MESSAGE_TYPE"];
        if (type == "REPL")
        {
            return repl::REPL_message::deserialize(message);
        }
        else if (type == "RPC")
        {
            // return rpc::RPC_message::deserialize(message);
            throw std::runtime_error("not implemented");
        }
        else
        {
            throw std::runtime_error("Unknown message type");
        }
    }
}
