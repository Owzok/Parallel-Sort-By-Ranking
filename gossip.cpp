#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
using namespace std;

// Concat strings en orden por rank
string concatenate(const map<int, string>& data_by_rank) {
    string result;
    for (const auto& entry : data_by_rank) {
        result += entry.second; 
    }
    return result;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int rows = 2;
    const int cols = 2; 

    if (size != rows * cols) {
        if (rank == 0) {
            cerr << "Error: This program requires exactly " << rows * cols << " processes." << endl;
        }
        MPI_Finalize();
        return 1;
    }

    int row = rank / cols;
    int col = rank % cols;

    map<int, string> local_data = {{rank, string(1, 'a' + rank) + string(1, 'a' + rank + 13)}};

    cout << "Process " << rank << " (Row " << row << ", Col " << col << ") starts with: " 
         << concatenate(local_data) << endl;

    char recv_buffer[200];

    for (int step = 0; step < rows - 1; ++step) {
        int send_to = ((row + 1) % rows) * cols + col;      // misma col pero abajo
        int receive_from = ((row + rows - 1) % rows) * cols + col; // arriba misma col

        string send_buffer = concatenate(local_data);

        MPI_Request send_request, recv_request;
        MPI_Irecv(recv_buffer, 200, MPI_CHAR, receive_from, 0, MPI_COMM_WORLD, &recv_request);
        MPI_Isend(send_buffer.c_str(), send_buffer.size() + 1, MPI_CHAR, send_to, 0, MPI_COMM_WORLD, &send_request);

        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        MPI_Wait(&send_request, MPI_STATUS_IGNORE);

        string received_string(recv_buffer);
        int source_rank = (rank - cols + size) % size; // basado en la pos, calcular el rank de quien envió la data
        local_data[source_rank] = received_string;
    }

    string result = concatenate(local_data);

    cout << "Process " << rank << " (Row " << row << ", Col " << col << ") ends with: " << result << endl;

    MPI_Finalize();
    return 0;
}