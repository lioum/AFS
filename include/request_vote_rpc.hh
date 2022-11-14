#include "rpc.hh"

class RequestVoteRPC : public RPC {
public:
    explicit RequestVoteRPC(const int term, const unsigned int candidate_id,
                            const unsigned int last_log_index,
                            const unsigned int last_log_term)
        : RPC(term), candidate_id(candidate_id),
          last_log_index(last_log_index), last_log_term(last_log_term) {});
          
    static std::shared_ptr<RequestVoteRPC>
    deserialize(const std::string &message);
    json serialize_json() const override;
private:
    const unsigned int candidate_id;
    const unsigned int last_log_index;
    const unsigned int last_log_term;
}; 