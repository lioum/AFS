#include <mpi.h>
#include <string>
#include "repl.hh"
#include "raft_server.hh"
#include "client.hh"

int main (int argc, char *argv[])
{
    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3 && rank == 0) 
        std::cout << "Use run.sh script." << std::endl;

    int nb_servers = std::stoi(argv[1]);
    int nb_clients = std::stoi(argv[2]);
    nb_clients = nb_clients;

    if (rank == 0)
    {
        //std::cout << rank << ": I'm the REPL" << std::endl;
        REPL repl(MPI_COMM_WORLD, nb_servers);
        repl.run();
    }
    else if (rank < nb_servers + 1)
    {
        //std::cout << rank << ": I'm a server" << std::endl;
        RaftServer server(MPI_COMM_WORLD, nb_servers, "server_folders/server_");
        server.run();
    }
    else
    {
        //std::cout << rank << ": I'm a client" << std::endl;
        Client client(MPI_COMM_WORLD, nb_clients, "client_folders/client_");
        client.run();
    }
    MPI_Finalize();
    
    return 0;
}
