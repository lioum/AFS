#include "repl_message.hh"
#include "message.hh"

#include <iostream>

namespace repl
{

    REPL_message::REPL_message(ReplType type, int target_rank)
        : Message(MessageType::REPL), repl_type(type), target_rank(target_rank), speed("")
    {}
    
    REPL_message::REPL_message(int target_rank, std::string speed)
        : Message(MessageType::REPL), repl_type(ReplType::SPEED), target_rank(target_rank), speed(speed)
    {}

    
    json REPL_message::serialize_json() const
    {
        json j;
        json data;
        data["REPL_TYPE"] = this->repl_type;
        data["TARGET"] = this->target_rank;
        j["MESSAGE_TYPE"] = MessageType::REPL;

        j["REPL"] = data;

        return j;
    }

    std::shared_ptr<REPL_message> REPL_message::deserialize(const std::string &message)
    {
       json j = json::parse(message); 
       json data = j["REPL"];
       ReplType type = static_cast<ReplType>(data["REPL_TYPE"]);
       auto bite = std::make_shared<REPL_message>(type, data["TARGET"]);

       return bite;
    }
}