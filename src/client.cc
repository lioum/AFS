#include "client.hh"

#include <chrono>
#include <thread>

#include "utils.hh"

Client Client::FromCommandFile(const std::filesystem::path &command_file_path,
                               MPI_Comm com, int nb_servers,
                               const std::filesystem::path &root_folder_path)
{
    Client client(com, nb_servers, root_folder_path);

    auto command_file_real_path = command_file_path.is_relative()
        ? client.working_folder_path / command_file_path
        : command_file_path;

    std::ifstream file(command_file_real_path);

    if (!file)
        throw std::runtime_error("Could not open command file");

    auto &messages = client.commands;
    auto target_rank = client.target_rank;
    int sender_rank = client.uid;

    // Parsing of the current command file that we previously receive or decided to get all the commands
    std::string line;

    while (std::getline(file, line))
    {
        std::vector<std::string> inline_words = split_words(line, ' ');

        std::cout << "exit while with target_alive: " << client.target_rank
                  << std::endl;

        if (inline_words[0] == "LOAD")
        {
            if (inline_words.size() < 2)
            {
                std::cout << "Filename required after LOAD in file: "
                          << command_file_real_path << std::endl;
                throw std::runtime_error("Failed to parse command file: " + command_file_real_path.string());
            }
            std::string filename = inline_words[1];

            // Read file content to content
            std::string content = readFileIntoString(filename);
            messages.push_back(std::make_unique<Load>(sender_rank, filename, content));
        }
        else if (inline_words[0] == "LIST")
        {
            if (inline_words.size() > 1)
            {
                std::cout << "No text required after LIST in file: " << command_file_real_path
                          << std::endl;
                throw std::runtime_error("Failed to parse command file: " + command_file_real_path.string());
            }
            messages.push_back(
                std::make_unique<List>(target_rank));
        }
        else if (inline_words[0] == "APPEND")
        {
            if (inline_words.size() < 3)
            {
                std::cout
                    << "uid of file and content required after APPEND in file: "
                    << command_file_real_path << std::endl;
                throw std::runtime_error("Failed to parse command file: " + command_file_real_path.string());
            }
            int uid_file = -1;
            try
            {
                uid_file = std::stoi(inline_words[1]);
                
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            messages.push_back(std::make_unique<Append>(sender_rank, uid_file, inline_words[2]));
        }
        else if (inline_words[0] == "DELETE")
        {
            if (inline_words.size() < 2)
            {
                std::cout << "UID of file required after DELETE in file: "
                          << command_file_real_path << std::endl;
                throw std::runtime_error("Failed to parse command file: " + command_file_real_path.string());
            }
            int uid_file = -1;
            try
            {
                uid_file = std::stoi(inline_words[1]);
                
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
            

            messages.push_back(std::make_unique<Delete>(sender_rank, uid_file));
        }
    }

    return client;
}

Client::Client(MPI_Comm com, int nb_servers,
               const std::filesystem::path &clients_root)
    : InternProcessus(com, nb_servers, clients_root)
{
}

void Client::work()
{
    // we check if the client is still available
    if (!started || crashed)
    {
        return;
    }
    std::chrono::milliseconds now =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

    // Looking for living target
    if (now > handshake_timeout_till)
    {
        std::cout << "Client " << uid
                  << " timed out waiting for handshake from server "
                  << target_rank << std::endl;
        const auto &cmd = commands.at(messages_index);
        ClientRequest request(-1, uid, cmd);

        broadcast_to_servers(request);
        waiting_for_handshake = true;
    }

    // Client ready to send a message 
    if (!waiting_for_handshake)
    {
        const auto &cmd = commands.at(messages_index);
        ClientRequest request(-1, uid, cmd);

        send(request);
        waiting_for_handshake = true;
        handshake_timeout_till = now + handshake_timeout;
    }

    int speed_val = as_integer(speed);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(speed_val * speed_val * 1000));
}

void Client::receive(HandshakeFailure &msg)
{
    // we check if the client is still available
    if (!started || crashed)
    {
        return;
    }

    // we check if the client is waiting for a message
    if (waiting_for_handshake)
    {
        std::cout << "Client " << uid << " received handshake failure from server "
              << msg.sender_rank <<std::endl;
        waiting_for_handshake = false;
    }
}

void Client::receive(MeNotLeader &msg)
{
    if (!started || crashed)
    {
        return;
    }

    if (waiting_for_handshake)
    {
        std::cout << "Client " << uid << " received MeNotLeader from server "
                  << msg.sender_rank << std::endl;
        std::cout << "Client " << uid
                  << " will use server " << msg.leader_uid << " instead of " << target_rank << " as target server" << std::endl;
        target_rank = msg.leader_uid;
        waiting_for_handshake = false;
    }
}

void Client::receive(HandshakeSuccess &msg)
{
    // we check if the client is still available
    if (!started || crashed)
    {
        return;
    }

    // we check if the client is waiting for a message
    if (waiting_for_handshake)
    {
        std::cout << "Client " << uid
                  << " received handshake success from server "
                  << msg.sender_rank << std::endl;
        
        waiting_for_handshake = false;
        messages_index += 1;
    }
}