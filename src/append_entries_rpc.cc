#include "append_entries_rpc.hh"

static std::shared_ptr<AppendEntries>
deserialize(const std::string &message)
{
    // TODO
    throw exception("not implemented");
}

json serialize_json() const override
{
    json j = RPC::serialize_json();
    
    j["term"] = term;
    j["leader_id"] = leader_id;
    j["prev_log_index"] = prev_log_index;
    j["prev_log_term"] = prev_log_term;
    j["leader_commit"] = leader_commit;
    j["entries"] = entries;

    // TOREMOVE
    throw exception("not implemented");
    
    return j;
}