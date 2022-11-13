#include "client_message.hh"

#include <iostream>

namespace message
{

    Client_message::Client_message(ClientAction action, int target_rank, int sender_rank)
        : Message(MessageType::CLIENT, sender_rank, target_rank)
        , action(action)
    {}

    
    json Client_message::serialize_json() const
    {
        json j;
        j["MESSAGE_TYPE"] = MessageType::CLIENT;
        j["SENDER"] = this->sender_rank;
        j["TARGET"] = this->target_rank;
        json data;
        data["ACTION"] = this->action;
        j["CLIENT"] = data;


        return j;
    }

    std::shared_ptr<Client_message> Client_message::deserialize(const std::string &message)
    {
       json j = json::parse(message); 
       ClientAction action = static_cast<ClientAction>(j["CLIENT"]["ACTION"]);
       return std::make_shared<Client_message>(action, j["TARGET"], j["SENDER"]);
    }
}