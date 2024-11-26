#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
using namespace std;

// Helper function to concatenate data in order of rank
string concatenate(const map<int, string>& data_by_rank) {
    string result;
    for (const auto& entry : data_by_rank) {
        result += entry.second; // Concatenate strings in rank order
    }
    return result;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int rows = 2; // Number of rows in the grid
    const int cols = 2; // Number of columns in the grid

    if (size != rows * cols) {
        if (rank == 0) {
            cerr << "Error: This program requires exactly " << rows * cols << " processes." << endl;
        }
        MPI_Finalize();
        return 1;
    }

    int row = rank / cols;
    int col = rank % cols;

    // Initial string data for each process
    string starting_data = string(1, 'a' + rank) + string(1, 'a' + rank + 13) + string(1, 'a' + rank + 5) + string(1, 'a' + rank + 17);
    map<int, string> local_data = {{rank, starting_data}};
    map<int, string> resulting_data = {};

    char recv_buffer[200]; // Buffer to receive data

    // Broadcast within the row (only the diagonal processes send data)
    if (col == row) {
        // If this is a diagonal process, send its data to the rest of the row
        for (int c = 0; c < cols; ++c) {
            if (c != col) { // Don't send to itself
                int send_to = row * cols + c; // Send to process in the same row, different column
                MPI_Send(starting_data.c_str(), starting_data.size() + 1, MPI_CHAR, send_to, 0, MPI_COMM_WORLD);
            }
        }
        resulting_data[0] = starting_data;
    } else {
        // If this is not a diagonal process, receive data from the diagonal process
        int send_from = row * cols + row; // The diagonal process to receive from
        MPI_Recv(recv_buffer, 200, MPI_CHAR, send_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        string received_string(recv_buffer);
        local_data[send_from] = received_string; // Store received data
        resulting_data[0] = received_string;
    }

    // Final concatenation in rank order
    string result = concatenate(resulting_data);

    // Print both starting data and the final result
    cout << "Process " << rank << " (Row " << row << ", Col " << col << ") starts with: " << starting_data << endl;
    cout << "Process " << rank << " (Row " << row << ", Col " << col << ") ends with: " << result << endl;

    MPI_Finalize();
    return 0;
}