#include "rpc.hh"


namespace rpc
{

    RPC::RPC(RPC_TYPES type, int target_rank, int sender_rank)
        : Message(MessageType::RPC, sender_rank, target_rank)
        , type(type)
    {}

    json RPC::serialize_json() const
    {
        
        auto j = Message::serialize_json();

        json data;

        data["TYPE"] = this->type;
        j["RPC"] = data;

        return j;
    }

    std::shared_ptr<RPC> RPC::deserialize(const std::string &message)
    {
        json j = json::parse(message);
        RPC_TYPES type = static_cast<RPC_TYPES>(j["RPC"]["TYPE"]);
        return std::make_shared<RPC>(type, j["TARGET"], j["SENDER"]);
    }

} // namespace rpc