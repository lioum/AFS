#include "handshake_message.hh"

#include <iostream>

namespace message
{

    Handshake_message::Handshake_message(HandshakeStatus status, int target_rank, int sender_rank)
        : Message(MessageType::REPL, sender_rank, target_rank)
        , status(status)
    {}

    Handshake_message::Handshake_message(HandshakeStatus status, int target_rank, int sender_rank, json custom_data)
        : Message(MessageType::REPL, sender_rank, target_rank)
        , status(status)
        , custom_data(custom_data)
    {}
    
    json Handshake_message::serialize_json() const
    {
        json j;
        j["MESSAGE_TYPE"] = MessageType::HANDSHAKE;
        j["SENDER"] = this->sender_rank;
        j["TARGET"] = this->target_rank;
        json data;
        data["STATUS"] = this->status;
        data["CUSTOM_DATA"] = this->custom_data;
        j["HANDSHAKE"] = data;


        return j;
    }

    std::shared_ptr<Handshake_message> Handshake_message::deserialize(const std::string &message)
    {
       json j = json::parse(message); 
       HandshakeStatus status = static_cast<HandshakeStatus>(j["HANDSHAKE"]["STATUS"]);
       auto bite = std::make_shared<Handshake_message>(status, j["TARGET"], j["SENDER"]);

       return bite;
    }
}