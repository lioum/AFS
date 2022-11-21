#include "raft_server.hh"

/* 
** Functions that implements raft servers behavior when executing a command
** Commands will be dynamically dispatched in thanks to visitor design pattern
*/
void RaftServer::execute(Load &msg)
{
    std::string filename = working_folder_path / msg.filename;

    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, filename.c_str(),
                  MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);
    MPI_File_write(file, msg.content.c_str(), msg.content.size(), MPI_CHAR,
                   MPI_STATUS_IGNORE);
    MPI_File_close(&file);

    // Save the server's UID in which the file is stored
    int file_uid = 0;
    while (uids.find(file_uid) != uids.end())
    {
        file_uid++;
    }
    uids[file_uid] = filename;

    // Only leader is capable of answering the client request
    if (role == Role::LEADER)
        send(SuccessLoad(msg.client_uid, uid, file_uid, msg.filename));
}

void RaftServer::execute(List &msg)
{
    // Save uids of stored files
    std::vector<int> list_uids;
    for (const auto &key_value : uids)
    {
        list_uids.push_back(key_value.first);
    }

    std::cerr << "RaftServer(" << uid << ") is going to list all files"
              << std::endl;

    // Only leader is capable of answering the client request
    if (role == Role::LEADER)
        send(SuccessList(msg.client_uid, uid, list_uids));
}

void RaftServer::execute(Append &msg)
{
    MPI_File file;
            
    auto filepath = uids[msg.uid];

    if(MPI_File_open(MPI_COMM_SELF, filepath.c_str(), MPI_MODE_APPEND | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &file) != MPI_SUCCESS)
    {
        std::cerr << "[MPI process " << uids[msg.uid] << "] Failure in opening the file.\n";
    }

    if(MPI_File_write(file, msg.content.c_str(), msg.content.size(), MPI_CHAR,
                   MPI_STATUS_IGNORE) != MPI_SUCCESS)
    {
        std::cerr << "[MPI process " << uids[msg.uid] << "] Failure in writing to the file.\n";
    }
    MPI_File_close(&file);

    std::cerr << "RaftServer(" << uid << ") is adding " << msg.content
              << " to file with uid " << msg.uid << std::endl;

    // Only leader is capable of answering the client request
    if (role == Role::LEADER)
        send(HandshakeSuccess(msg.client_uid, uid));
}

void RaftServer::execute(Delete &msg)
{
    MPI_File_delete(uids[msg.uid].c_str(), MPI_INFO_NULL);
    uids.erase(msg.uid);

    std::cerr << "RaftServer(" << uid << ") is deleting file with uid "
              << msg.uid << std::endl;

    // Only leader is capable of answering the client request
    if (role == Role::LEADER)
        send(HandshakeSuccess(msg.client_uid, uid));
}