class Receiver
{
    virtual void receive(ReplCrash &msg);
    virtual void receive(ReplSpeed &msg);
    virtual void receive(ReplStart &msg);
    virtual void receive(RpcRequestVote &msg);
    virtual void receive(RpcAppendEntries &msg);
    virtual void receive(RpcVoteResponse &msg);
    virtual void receive(RpcAppendEntriesResponse &msg);
    virtual void receive(HandshakeFailure &msg);
    virtual void receive(HandshakeSuccess &msg);
    virtual void receive(ClientLoad &msg);
    virtual void receive(ClientList &msg);
    virtual void receive(ClientAppend &msg);
    virtual void receive(ClientDelete &msg);
};