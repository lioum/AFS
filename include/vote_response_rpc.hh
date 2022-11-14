#include "rpc.hh"

class VoteResponseRPC : public RPC {
public:
    explicit VoteResponseRPC(const int term, const bool vote_granted)
        : RPC(term), candidate_id(candidate_id),
          last_log_index(last_log_index), last_log_term(last_log_term)
          {}
          
    static std::shared_ptr<VoteResponseRPC>
    deserialize(const std::string &message);
    json serialize_json() const override;
private:
    const bool vote_granted;
}; 