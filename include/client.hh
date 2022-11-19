#pragma once

#include "processus.hh"

/*
** Class Client
**
** Instance of a client that will receive commands from the server
*/

class Client : public InternProcessus
{
public:

    /*
    ** Constructor
    **
    ** @MPI_Comm com : the communicator provided by MPI that contains all MPI processes
    ** @int nb_servers : the number of servers
    ** @std::filesystem::path &working_path : the reference the current working environment for the processus
    */
    Client(MPI_Comm com, int nb_servers,
           const std::filesystem::path &working_path);
    ~Client(){};

    /*
    ** FromCommandFile Function
    **
    ** @std::filesystem::path &command_file_path : the reference to the command file
    ** @MPI_Comm com : the communicator provided by MPI that contains all MPI processes
    ** @int nb_servers : the number of servers
    ** @std::filesystem::path &folder_path : the reference to the folder of the processus
    */
    static Client
    FromCommandFile(const std::filesystem::path &command_file_path,
                    MPI_Comm com, int nb_servers,
                    const std::filesystem::path &folder_path);

    /*
    ** work Function
    **
    ** The client will spread to the servers his messages and wait for an answer
    */
    void work() override;

    /*
    ** receive Function
    **
    ** case HandshakeFailure : receive a message of failure and doesnt keep it
    ** case HandshakeSuccess : receive a message of success and keep it
    */
    void receive(HandshakeFailure &msg) override;
    void receive(HandshakeSuccess &msg) override;
    
    void receive(MeNotLeader &msg) override;

private:
    size_t messages_index = 0;
    int target_rank = -1;                                   // Index of the current target server
    bool target_alive = false;                              // Satus of the current target server
    bool waiting_for_handshake = false;                     // Status of the current client
    int expected_handshake_rank = 0;

    std::chrono::milliseconds handshake_timeout =
        std::chrono::milliseconds(10000);
    std::chrono::milliseconds handshake_timeout_till =
        std::chrono::milliseconds(0);
    std::vector<std::shared_ptr<Command>> commands;
};