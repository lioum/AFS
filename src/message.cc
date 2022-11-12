#include "message.hh"
#include <memory>
#include <iostream>
#include "repl_message.hh"
#include "handshake_message.hh"

namespace message
{
    Message::Message(MessageType type, int sender_rank, int target_rank)
        : type(type)
        , sender_rank(sender_rank)
        , target_rank(target_rank)
    {}

    std::string Message::serialize() const
    {
        json j;
        switch (type)
        {
        case MessageType::REPL:
            j = this->serialize_json();
            break;
        case MessageType::RPC:
            j = this->serialize_json();
            break;
        case MessageType::HANDSHAKE:
            j = this->serialize_json();
            break;
        default:
            throw std::runtime_error("Unknown message type");
        }
        
        return j.dump();
    }

    std::shared_ptr<Message> Message::deserialize(const std::string &message)
    {
        json j = json::parse(message);
        MessageType type = static_cast<MessageType>(j["MESSAGE_TYPE"]);
        if (type == MessageType::REPL)
        {
          return repl::REPL_message::deserialize(message);
        }
        else if (type == MessageType::RPC) // RPC
        {
            // return rpc::RPC_message::deserialize(message);
            throw std::runtime_error("not implemented");
        }
        else if (type == MessageType::HANDSHAKE)
        {
            return Handshake_message::deserialize(message);
        }
        else
        {
            throw std::runtime_error("Unknown message type");
        }
    }
}
