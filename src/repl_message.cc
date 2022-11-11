#include "repl_message.hh"

namespace repl
{

    REPL_message::REPL_message(ReplType type, int target_rank)
        : Message(MessageType::REPL), repl_type(type), target_rank(target_rank)
    {}

    static std::shared_ptr<REPL_message> deserialize(const std::string &message)
    {
       auto json = json::parse(message); 
       auto data = json["REPL"];

       auto type = data["REPL_TYPE"];
       auto sus = data["TARGET"];
       auto bite = std::make_shared<REPL_message>(type, sus);

       return bite;
    }
}