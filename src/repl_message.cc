#include "repl_message.hh"
#include "message.hh"

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
        json json;
        json["REPL_TYPE"] = this->repl_type;
        json["TARGET"] = this->target_rank;
        return json;
    }

    std::shared_ptr<REPL_message> REPL_message::deserialize(const std::string &message)
    {
       auto json = json::parse(message); 
       auto data = json["REPL"];

       ReplType type = data["REPL_TYPE"];
       int sus = data["TARGET"];
       auto bite = std::make_shared<REPL_message>(type, sus);

       return bite;
    }
}