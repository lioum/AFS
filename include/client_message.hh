#pragma once

#include <memory>
#include "message.hh"

namespace message
{
    enum ClientAction
    {
        LOAD = 0,
        LIST,
        APPEND,
        DELETE,
    };
    
    class Client_message : public Message
    {
    public:
        Client_message(ClientAction action, int target_rank, int sender_rank);


        static std::shared_ptr<Client_message>
        deserialize(const std::string &message);
        virtual json serialize_json() const;
    private:
        ClientAction action;
    };
}
