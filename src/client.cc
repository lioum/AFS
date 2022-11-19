#include "client.hh"

#include <chrono>
#include <thread>

#include "utils.hh"

// Client Client::FromCommandFile(const std::filesystem::path
// &command_file_path,
//                                MPI_Comm com, int nb_servers,
//                                const std::filesystem::path &root_folder_path)
// {
//     Client client(com, nb_servers, root_folder_path);

//     auto command_file_real_path = command_file_path.is_relative()
//         ? client.working_folder_path / command_file_path
//         : command_file_path;

//     client.commands_file = std::ifstream(command_file_real_path);

//     if (!client.commands_file)
//         throw std::runtime_error("Could not open command file");

// auto &messages = client.commands;
// auto target_rank = client.target_rank;
// auto &command_index = client.command_index;
// int sender_rank = client.uid;

// // Parsing of the current command file that we previously receive or decided
// to get all the commands std::string line;

// while (std::getline(file, line))
// {
//     std::vector<std::string> inline_words = split_words(line, ' ');

//     std::cerr << "exit while with target_alive: " << client.target_rank
//               << std::endl;

//     if (inline_words[0] == "LOAD")
//     {
//         if (inline_words.size() < 2)
//         {
//             std::cerr << "Filename required after LOAD in file: "
//                       << command_file_real_path << std::endl;
//             throw std::runtime_error("Failed to parse command file: " +
//             command_file_real_path.string());
//         }
//         std::string filename = inline_words[1];

//         // Read file content to content
//         std::string content = readFileIntoString(filename);
//         messages.push_back(std::make_unique<Load>(sender_rank,
//         command_index++, filename, content));
//     }
//     else if (inline_words[0] == "LIST")
//     {
//         if (inline_words.size() > 1)
//         {
//             std::cerr << "No text required after LIST in file: " <<
//             command_file_real_path
//                       << std::endl;
//             throw std::runtime_error("Failed to parse command file: " +
//             command_file_real_path.string());
//         }
//         messages.push_back(
//             std::make_unique<List>(sender_rank, command_index++));
//     }
//     else if (inline_words[0] == "APPEND")
//     {
//         if (inline_words.size() < 3)
//         {
//             std::cerr
//                 << "uid of file and content required after APPEND in file: "
//                 << command_file_real_path << std::endl;
//             throw std::runtime_error("Failed to parse command file: " +
//             command_file_real_path.string());
//         }
//         std::string filename = inline_words[1];
//         messages.push_back(std::make_unique<Append>(sender_rank,
//         command_index++, uid_file, inline_words[2]));
//     }
//     else if (inline_words[0] == "DELETE")
//     {
//         if (inline_words.size() < 2)
//         {
//             std::cerr << "UID of file required after DELETE in file: "
//                       << command_file_real_path << std::endl;
//             throw std::runtime_error("Failed to parse command file: " +
//             command_file_real_path.string());
//         }
//         int uid_file = -1;
//         try
//         {
//             uid_file = std::stoi(inline_words[1]);

//         }
//         catch(const std::exception& e)
//         {
//             std::cerr << e.what() << '\n';
//         }

//         messages.push_back(std::make_unique<Delete>(sender_rank,
//         command_index++, uid_file));
//     }
// }

//     return client;
// }

std::unique_ptr<Command> Client::load_next_command()
{
    // Client client(com, nb_servers, root_folder_path);

    // auto command_file_real_path = command_file_path.is_relative()
    //     ? client.working_folder_path / command_file_path
    //     : command_file_path;

    // commands_file = std::ifstream(command_file_real_path);

    // if (!commands_file)
    //     throw std::runtime_error("Could not open command file");

    // auto &messages = client.commands;
    // auto target_rank = client.target_rank;

    // Parsing of the current command file that we previously receive or decided
    // to get all the commands
    std::string line;

    while (std::getline(commands_file, line))
    {
        std::vector<std::string> inline_words = split_words(line, ' ');

        // std::cerr << "exit while with target_alive: " << client.target_rank
        //           << std::endl;

        if (inline_words[0] == "LOAD")
        {
            if (inline_words.size() < 2)
            {
                std::cerr << "Command file of Client(" << uid << "): "
                          << "Filename required after LOAD" << std::endl;

                throw std::runtime_error("Failed to parse command file");
            }
            std::string filename = inline_words[1];

            // Read file content to content
            std::string content = readFileIntoString(filename);
            // messages.push_back(std::make_unique<Load>(sender_rank,
            // command_index++, filename, content));
            return std::make_unique<Load>(uid, command_index++, filename,
                                          content);
        }
        else if (inline_words[0] == "LIST")
        {
            if (inline_words.size() > 1)
            {
                std::cerr << "Command file of Client(" << uid << "): "
                          << "No text required after LIST" << std::endl;
                throw std::runtime_error("Failed to parse command file");
            }
            // messages.push_back(
            //     std::make_unique<List>(sender_rank, command_index++));
            return std::make_unique<List>(uid, command_index++);
        }
        else if (inline_words[0] == "APPEND")
        {
            if (inline_words.size() < 3)
            {
                std::cerr << "Command file of Client(" << uid << "): "
                          << "UID of file and content required after APPEND"
                          << std::endl;
                throw std::runtime_error("Failed to parse command file");
            }
            auto uid_file = filename_to_uid[inline_words[1]];
            // messages.push_back(std::make_unique<Append>(sender_rank,
            // command_index++, uid_file, inline_words[2]));
            return std::make_unique<Append>(uid, command_index++, uid_file,
                                            inline_words[2]);
        }
        else if (inline_words[0] == "DELETE")
        {
            if (inline_words.size() < 2)
            {
                std::cerr << "Command file of Client(" << uid << "): "
                          << "UID of file required after DELETE" << std::endl;
                throw std::runtime_error("Failed to parse command file");
            }

            auto uid_file = filename_to_uid[inline_words[1]];

            // messages.push_back(std::make_unique<Delete>(sender_rank,
            // command_index++, uid_file));
            return std::make_unique<Delete>(uid, command_index++, uid_file);
        }
    }

    return nullptr;
}

Client::Client(
    MPI_Comm com, int nb_servers, const std::filesystem::path &root_folder_path,
    const std::filesystem::path &command_file_path)
    : InternProcessus(com, nb_servers, root_folder_path)
{
    auto command_file_real_path = command_file_path.is_relative()
        ? working_folder_path / command_file_path
        : command_file_path;

    commands_file = std::ifstream(command_file_real_path);

    if (!commands_file)
        throw std::runtime_error("Could not open command file");
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

    // Looking for new leader
    if (now > handshake_timeout_till)
    {
        std::cerr << "Client " << uid
                  << " timed out waiting for handshake from server "
                  << target_rank << std::endl;
        // const auto &cmd = commands.at(messages_index);

        target_rank = rand() % nb_servers + 1;

        ClientRequest request(target_rank, uid, current_command);

        //broadcast_to_servers(request);
        send(request);
        waiting_for_handshake = true;
    }

    // Client ready to send a message
    if (!waiting_for_handshake)
    {
        // const auto &cmd = commands.at(messages_index);
        ClientRequest request(-1, uid, current_command);

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
        std::cerr << "Client " << uid
                  << " received handshake failure from server "
                  << msg.sender_rank << std::endl;
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
        std::cerr << "Client " << uid << " received MeNotLeader from server "
                  << msg.sender_rank << std::endl;
        std::cerr << "Client " << uid << " will use server " << msg.leader_uid
                  << " instead of " << target_rank << " as target server"
                  << std::endl;
        target_rank = msg.leader_uid;
        waiting_for_handshake = false;
    }
}

void Client::receive(SuccessLoad &msg)
{
    if (!started || crashed)
    {
        return;
    }

    if (waiting_for_handshake)
    {
        filename_to_uid[msg.file_name] = msg.file_uid;
        waiting_for_handshake = false;
    }
}

void Client::receive(SuccessList &msg)
{
    if (!started || crashed)
    {
        return;
    }
    // jsp pas quoi faire
    msg = msg;

    if (waiting_for_handshake)
    {
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
        std::cerr << "Client " << uid
                  << " received handshake success from server "
                  << msg.sender_rank << std::endl;

        waiting_for_handshake = false;
        // messages_index += 1;
        current_command = load_next_command();
    }
}