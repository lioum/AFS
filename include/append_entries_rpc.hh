#include "rpc.hh"

class AppendEntries : public RPC {
public:
    explicit AppendEntries(const int term, const unsigned int leader_id,
                            const unsigned int prev_log_index,
                            const unsigned int prev_log_term,
                            const unsigned int leader_commit,
                            const std::vector<LogEntry> &entries)
        : RPC(term), leader_id(leader_id),
        prev_log_index(prev_log_index), prev_log_term(prev_log_term)
        {}
          
    static std::shared_ptr<AppendEntries>
    deserialize(const std::string &message);
    json serialize_json() const override;
private:
    const unsigned int leader_id;
    const unsigned int prev_log_index;
    const unsigned int prev_log_term;
    const unsigned int leader_commit;
    const std::vector<LogEntry> entries;
}; 