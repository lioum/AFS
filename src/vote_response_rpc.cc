#include "vote_response_rpc.hh"

static std::shared_ptr<VoteResponseRPC>
deserialize(const std::string &message)
{
    // TODO
    throw exception("not implemented");
}

json serialize_json() const override
{
    json j = RPC::serialize_json();
    
    j["term"] = term;
    j["vote_granted"] = vote_granted;

    // TOREMOVE
    throw exception("not implemented");
    
    return j;
}