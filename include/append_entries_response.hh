#include "rpc.hh"

class AppendEntriesResponse : public RPC {
public:
    explicit AppendEntriesResponse(const int term, const bool successs)
        : RPC(term), success(success),
          {}
          
    static std::shared_ptr<AppendEntriesResponse>
    deserialize(const std::string &message);
    json serialize_json() const override;
private:
    const bool success;
}; 