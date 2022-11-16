#pragma once

#include "processus.hh"

class Client : public InternProcessus
{
public:
    Client(MPI_Comm com, int nb_servers, const std::filesystem::path &working_path);
    
    static Client FromCommandFile(const std::filesystem::path &command_file_path, MPI_Comm com, int nb_servers, const std::filesystem::path &folder_path);

    void work() override;

    void receive(HandshakeFailure &msg) override;
    void receive(HandshakeSuccess &msg) override;

private:
    int target_rank;
    bool target_alive = false;
    size_t messages_index;
    bool waiting_for_handshake = false;
    int expected_handshake_rank = 0;

    std::chrono::milliseconds handshake_timeout = std::chrono::milliseconds(1000);
    std::chrono::milliseconds handshake_timeout_till = std::chrono::milliseconds(0);
    std::vector<std::shared_ptr<ClientMessage>> messages;
};