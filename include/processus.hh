#pragma once

class Processus {
    
public:
    Processus(MPI_Comm com);

    std::shared_ptr<message::Message> listen();

    void send(int target_rank, std::shared_ptr<message::Message> message);

    virtual void on_message_callback(std::shared_ptr<message::Message> message) = 0;

    void run();
    
    virtual void work() = 0;
    
    MPI_Comm com;

    virtual void receive(Message &msg) {return;}
};
