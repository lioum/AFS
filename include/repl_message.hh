#pragma once

#include <memory>
#include "message.hh"

using namespace message;

namespace repl
{
    enum ReplType
    {
        CRASH = 0,
        SPEED,
        START,
    };

    enum ReplSpeed
    {
        NONE = 0,
        LOW,
        MEDIUM,
        FAST,
    };

    class REPL_message : public Message
    {
    public:
        explicit REPL_message(ReplType type, int target_rank, int sender_rank);
        REPL_message(int target_rank, int sender_rank, ReplSpeed speed);

        static std::shared_ptr<REPL_message>
        deserialize(const std::string &message);
        virtual json serialize_json() const;
    private:
        ReplType repl_type;
        ReplSpeed speed;
    };
}
