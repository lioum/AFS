#include "client.hh"

#include <chrono>
#include <thread>

#include "utils.hh"

// Reads a line from the file containing the commands and processes it as a
// Command object.
std::unique_ptr<Command> Client::load_next_command()
{
    // Parsing of the current command file that we previously receive or decided
    // to get all the commands
    std::string line;

    while (std::getline(commands_file, line))
    {
        std::vector<std::string> inline_words = split_words(line, ' ');

        if (inline_words[0] == "LOAD")
        {
            if (inline_words.size() < 2)
            {
                std::cerr << client_str << " Filename required after LOAD"
                          << std::endl;

                throw std::runtime_error("Failed to parse command file");
            }
            auto filepath = working_folder_path / inline_words[1];

            // Read file content to content
            std::string content = readFileIntoString(filepath);
            return std::make_unique<Load>(uid, command_index++, inline_words[1],
                                          content);
        }
        else if (inline_words[0] == "LIST")
        {
            if (inline_words.size() > 1)
            {
                std::cerr << client_str << " No text required after LIST"
                          << std::endl;
                throw std::runtime_error("Failed to parse command file");
            }

            return std::make_unique<List>(uid, command_index++);
        }
        else if (inline_words[0] == "APPEND")
        {
            if (inline_words.size() < 3)
            {
                std::cerr
                    << client_str
                    << " UID of file and content are both required after APPEND"
                    << std::endl;
                throw std::runtime_error("Failed to parse command file");
            }

            try
            {
                auto uid_file = inline_words[1];
                return std::make_unique<Append>(uid, command_index++, uid_file,
                                                inline_words[2]);
            }
            catch (std::exception &error)
            {
                std::cerr << client_str << " File " << inline_words[1]
                          << " is not loaded (in APPEND command)" << std::endl;
            }
        }
        else if (inline_words[0] == "DELETE")
        {
            if (inline_words.size() < 2)
            {
                std::cerr << client_str << " UID of file required after DELETE"
                          << std::endl;
                throw std::runtime_error("Failed to parse command file");
            }

            try
            {
                auto uid_file = inline_words[1];
                return std::make_unique<Delete>(uid, command_index++, uid_file);
            }
            catch (std::exception &error)
            {
                std::cerr << client_str << " File " << inline_words[1]
                          << " is not loaded (in DELETE command)" << std::endl;
            }
        }
    }

    return nullptr;
}

Client::Client(int nb_servers, const std::filesystem::path &root_folder_path,
               const std::filesystem::path &command_file_path)
    : InternProcessus(nb_servers, root_folder_path)
{
    auto command_file_real_path = command_file_path.is_relative()
        ? working_folder_path / command_file_path
        : command_file_path;

    commands_file = std::ifstream(command_file_real_path);

    if (!commands_file)
        throw std::runtime_error("Could not open command file");

    current_command = load_next_command();
    client_str = "[Client " + std::to_string(uid) + "]";
}

void Client::work()
{
    // we check if the client is still available
    if (!started || crashed || current_command == nullptr)
        return;

    std::chrono::milliseconds now =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

    // Looking for new leader
    if (now > handshake_timeout_till)
    {
        std::cerr << client_str
                  << " timed out waiting for handshake from server "
                  << target_rank << std::endl;

        target_rank = rand() % nb_servers + 1;
        ClientRequest request(target_rank, uid, current_command);

        // broadcast_to_servers(request);
        send(request);
        waiting_for_handshake = true;
        handshake_timeout_till = now + handshake_timeout;
    }

    // Client ready to send a message
    if (!waiting_for_handshake)
    {
        ClientRequest request(target_rank, uid, current_command);

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
        std::cerr << client_str << " received handshake failure from server "
                  << msg.sender_rank << std::endl;
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
        std::cerr << client_str << " received handshake success from server "
                  << msg.sender_rank << std::endl;

        waiting_for_handshake = false;
        current_command = load_next_command();
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
        std::cerr << client_str << " Received MeNotLeader redirecting to "
                  << msg.leader_uid << " from server " << msg.sender_rank
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
        std::cerr << client_str << " received LOAD success from server "
                  << msg.sender_rank << std::endl;

        waiting_for_handshake = false;
        current_command = load_next_command();
    }
}

void Client::receive(SuccessList &msg)
{
    if (!started || crashed)
    {
        return;
    }

    if (waiting_for_handshake)
    {
        std::cerr << client_str << " Received LIST success from server "
                  << msg.sender_rank << std::endl;

        std::cerr << client_str << " UIDs list:" << std::endl;

        for (const auto &uid : msg.file_uids)
            std::cerr << "\t- " << uid << std::endl;

        waiting_for_handshake = false;
        current_command = load_next_command();
    }
}
