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
    Client(MPI_Comm com, int nb_servers,
           const std::filesystem::path &working_path);
    ~Client(){};

    static Client
    FromCommandFile(const std::filesystem::path &command_file_path,
                    MPI_Comm com, int nb_servers,
                    const std::filesystem::path &folder_path);

    void work() override;

    void receive(HandshakeFailure &msg) override;
    void receive(HandshakeSuccess &msg) override;

private:
    size_t messages_index = 0;
    int target_rank = -1;
    bool target_alive = false;
    bool waiting_for_handshake = false;
    int expected_handshake_rank = 0;

    std::chrono::milliseconds handshake_timeout =
        std::chrono::milliseconds(1000);
    std::chrono::milliseconds handshake_timeout_till =
        std::chrono::milliseconds(0);
    std::vector<std::shared_ptr<ClientRequest>> messages;
};