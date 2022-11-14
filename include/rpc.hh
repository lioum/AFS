#include "message.hh"

namespace raft
{
    enum class RPC_TYPES
    {
        REQUEST_VOTE_RPC,
        APPEND_ENTRIES_RPC,
        VOTE_RESPONSE_RPC,
        APPEND_ENTRIES_RESPONSE_RPC,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(
        RPC_TYPES,
        {
            { RPC_TYPES::REQUEST_VOTE_RPC, "REQUEST_VOTE_RPC" },
            { RPC_TYPES::APPEND_ENTRIES_RPC, "APPEND_ENTRIES_RPC" },
            { RPC_TYPES::VOTE_RESPONSE_RPC, "VOTE_RESPONSE_RPC" },
            { RPC_TYPES::APPEND_ENTRIES_RESPONSE_RPC,
              "APPEND_ENTRIES_RESPONSE_RPC" },
        })

    // Remote Procedure Call
    class RPC : public Message
    {
    public:
        explicit RPC(RPC_TYPES type, int target_rank, int sender_rank);

        static std::shared_ptr<RPC>
        deserialize(const std::string &message);
        virtual json serialize_json() const;

    private:

        RPC_TYPES rpc_type;
        const unsigned int term;

    };
} // namespace raft
