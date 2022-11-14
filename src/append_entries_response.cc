#include "append_entries_response.hh"

static std::shared_ptr<AppendEntriesResponse>
deserialize(const std::string &message)
{
    // TODO
    throw exception("not implemented");
}

json serialize_json() const override
{
    json j = RPC::serialize_json();
    
    j["term"] = term;
    j["success"] = success;

    // TOREMOVE
    throw exception("not implemented");
    
    return j;
}