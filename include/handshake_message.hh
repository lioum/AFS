#pragma once

#include <memory>
#include "message.hh"

namespace message
{
    enum HandshakeStatus
    {
        SUCCESS = 0,
        FAILURE,
    };
    
    class Handshake_message : public Message
    {
    public:
        Handshake_message(HandshakeStatus status, int target_rank, int sender_rank);


        static std::shared_ptr<Handshake_message>
        deserialize(const std::string &message);
        virtual json serialize_json() const;
    private:
        HandshakeStatus status;
    };
}
