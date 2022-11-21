#pragma once

#include "processus.hh"

#include <fstream>
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
    ** @int nb_servers : the number of servers
    ** @std::filesystem::path &working_path : the reference the current working
    *environment for the processus
    */
    Client(int nb_servers,
           const std::filesystem::path &root_folder_path,
           const std::filesystem::path &command_file_path);
    ~Client(){};

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
    void receive(SuccessLoad &msg) override;
    void receive(SuccessList &msg) override;

    std::unique_ptr<Command> load_next_command();

private:
    int command_index = 0;
    int target_rank = -1; // Index of the current target server
    bool waiting_for_handshake = false; // Status of the current client
    int expected_handshake_rank = 0;

    std::map<std::string, int> filename_to_uid;

    std::shared_ptr<Command> current_command;

    std::ifstream commands_file;

    std::chrono::milliseconds handshake_timeout =
        std::chrono::milliseconds(1000);
    std::chrono::milliseconds handshake_timeout_till =
        std::chrono::milliseconds(0);
    std::string client_str;
    // std::vector<std::shared_ptr<Command>> commands;
};