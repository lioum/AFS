#include "request_vote_rpc.hh"

static std::shared_ptr<RequestVoteRPC>
deserialize(const std::string &message)
{
    // TODO
    throw exception("not implemented");
}

json serialize_json() const override
{
    json j = RPC::serialize_json();
    
    j["term"] = term;
    j["candidate_id"] = candidate_id;
    j["last_log_index"] = last_log_index;
    j["last_log_term"] = last_log_term;

    // TOREMOVE
    throw exception("not implemented");
    
    return j;
}