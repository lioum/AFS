#include "repl_message.hh"
#include "message.hh"

#include <iostream>

namespace repl
{

    REPL_message::REPL_message(ReplType type, int target_rank, int sender_rank)
        : Message(MessageType::REPL, sender_rank, target_rank), repl_type(type), speed(ReplSpeed::NONE)
    {}
    
    REPL_message::REPL_message(int target_rank, int sender_rank, ReplSpeed speed)
        : Message(MessageType::REPL, sender_rank, target_rank), repl_type(ReplType::SPEED), speed(speed)
    {}

    
    json REPL_message::serialize_json() const
    {
        json j;
        json data;
        data["REPL_TYPE"] = this->repl_type;
        data["SPEED"] = this->speed;
        j["MESSAGE_TYPE"] = MessageType::REPL;
        j["SENDER"] = this->sender_rank;
        j["TARGET"] = this->target_rank;

        j["REPL"] = data;

        return j;
    }

    std::shared_ptr<REPL_message> REPL_message::deserialize(const std::string &message)
    {
       json j = json::parse(message); 
       json data = j["REPL"];
       ReplType type = static_cast<ReplType>(data["REPL_TYPE"]);
       auto bite = std::make_shared<REPL_message>(type, j["TARGET"], j["SENDER"]);

       return bite;
    }
}