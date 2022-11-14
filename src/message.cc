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

    std::string Message::serialize_json() const
    {
        json j;

        j["MESSAGE_TYPE"] = this->type;

        j["SENDER"] = this->sender_rank;
        j["TARGET"] = this->target_rank;
        
        return j;
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
            return raft::RPC::deserialize(message);
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
