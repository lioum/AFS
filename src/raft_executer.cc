#include "raft_server.hh"

/* 
** Functions that implements raft servers behavior when executing a command
** Commands will be dynamically dispatched in thanks to visitor design pattern
*/
void RaftServer::execute(Load &msg)
{
    std::string filepath = working_folder_path / msg.filename;

    auto [insertion, succes] = file_map.emplace(msg.filename, MPI_File{});
    auto &file_handle = insertion->second;

    if (succes)
    {
        if (MPI_File_open(MPI_COMM_SELF, filepath.c_str(),
                          MPI_MODE_CREATE | MPI_MODE_WRONLY | MPI_MODE_EXCL
                              | MPI_MODE_APPEND | MPI_MODE_DELETE_ON_CLOSE,
                          MPI_INFO_NULL, &insertion->second)
            != MPI_SUCCESS)
        {
            MPI_File_delete(filepath.c_str(), MPI_INFO_NULL);

            if (MPI_File_open(MPI_COMM_SELF, filepath.c_str(),
                              MPI_MODE_CREATE | MPI_MODE_WRONLY | MPI_MODE_EXCL
                                  | MPI_MODE_APPEND | MPI_MODE_DELETE_ON_CLOSE,
                              MPI_INFO_NULL, &file_handle)
                != MPI_SUCCESS)
                std::cerr << server_str << " Could not open file " << filepath << std::endl;
        }
    }
    else
    {
        MPI_File_close(&insertion->second);

        if (MPI_File_open(MPI_COMM_SELF, filepath.c_str(),
                          MPI_MODE_CREATE | MPI_MODE_WRONLY | MPI_MODE_EXCL
                              | MPI_MODE_APPEND | MPI_MODE_DELETE_ON_CLOSE,
                          MPI_INFO_NULL, &file_handle)
            != MPI_SUCCESS)
            std::cerr << server_str << " Could not reopen file " << filepath << std::endl;
    }

    MPI_File_write(file_handle, msg.content.c_str(), msg.content.size(),
                   MPI_CHAR, MPI_STATUS_IGNORE);

    if (role == Role::LEADER)
        send(SuccessLoad(msg.client_uid, uid, msg.filename));
}

void RaftServer::execute(List &msg)
{
    // Save uids of stored files
    std::vector<std::string> list_uids;

    for (const auto &[file_uid, _] : file_map)
        list_uids.push_back(file_uid);

    std::cerr << server_str << " Send list of all files"
              << std::endl;

    // Only leader is capable of answering the client request
    if (role == Role::LEADER)
        send(SuccessList(msg.client_uid, uid, list_uids));
}

void RaftServer::execute(Append &msg)
{
    if (auto search = file_map.find(msg.uid); search != file_map.end())
    {
        auto content = "\n" + msg.content;
        int result =
            MPI_File_write(search->second, content.c_str(), content.size(),
                           MPI_CHAR, MPI_STATUS_IGNORE);

        if (result != MPI_SUCCESS)
        {
            std::cerr << server_str << " [MPI process " << search->first
                      << "] Failure in writing to the file.\n";
        }
        // Only leader is capable of answering the client request
        else if (role == Role::LEADER)
            send(HandshakeSuccess(msg.client_uid, uid));
    }
}

void RaftServer::execute(Delete &msg)
{

    if (auto search = file_map.find(msg.uid); search != file_map.end())
    {
        MPI_File_close(&search->second);

        file_map.erase(search);

        std::cerr << server_str << " Deleting file with uid "
                  << msg.uid << std::endl;

    // Only leader is capable of answering the client request
        if (role == Role::LEADER)
            send(HandshakeSuccess(msg.client_uid, uid));
    }
}