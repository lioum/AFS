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

    class REPL_message : public Message
    {
    public:
        explicit REPL_message(ReplType type, int target_rank);
        REPL_message(int target_rank, std::string speed);

        int get_target_rank() const { return target_rank; }

        static std::shared_ptr<REPL_message>
        deserialize(const std::string &message);
        virtual json serialize_json() const;
    private:
        ReplType repl_type;
        int target_rank;
        std::string speed;
    };
}
