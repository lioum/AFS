#pragma once

#include <memory>
#include "message.hh"

using namespace message;

namespace repl
{
    enum class ReplType
    {
        CRASH,
        SPEED,
        START,
    };

    class REPL_message : public Message
    {
    public:
        explicit REPL_message(ReplType type, int target_rank);

        ReplType repl_type;
        int target_rank;

        static std::shared_ptr<REPL_message>
        deserialize(const std::string &message);
    private:
        virtual json serialize_json() const;
    };
}
