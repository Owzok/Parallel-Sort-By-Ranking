#include <map>
#include <mpi.h>
#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
using namespace std;

/**
 * @brief Concatena las strings asociadas con cada rank en el map.
 *
 * Concatena las strings en el orden de los ranks. El menor rank sería la primera string y el mayor el último.
 *
 * @param data_by_rank Un mapa donde las llaves son los rangos y los valores las stings.
 * @return Una string que sea la concatenación adecuada.
 */
string concatenar(const map<int, string>& data_by_rank) {
    string result;
    for (const auto& entry : data_by_rank) {
        result += entry.second;
    }
    return result;
}

/**
 * @brief Calcula el rango local de cada carácter en la cadena A comparado con la cadena local_A.
 *
 * Este método calcula el número de caracteres en la cadena local_A que son menores que cada carácter en la cadena A.
 * El resultado es un vector de enteros, donde cada elemento representa el rango local del carácter correspondiente en la cadena A.
 *
 * @param local_A La cadena local que se utilizará para comparar con cada carácter en la cadena A.
 * @param A La cadena completa que se utilizará para comparar con cada carácter en la cadena local_A.
 * @return Un vector de enteros que representa el rango local de cada carácter en la cadena A.
 */
vector<int> local_rank(const string& local_A, const string& A) {
    vector<int> rank_counts(A.size(), 0);

    for (size_t i = 0; i < A.size(); i++) {
        for (size_t j = 0; j < local_A.size(); j++) {
            if (local_A[j] <= A[i]) {
                rank_counts[i]++;
            }
        }
    }

    return rank_counts;
}

/**
 * @brief Realiza el paso de "Gossip".
 *
 * La función simula un protocolo "Gossip" donde cada proceso envía su data a los procesos vecinos de forma vertical.
 * La función actualiza los datos locales de cada proceso con los datos recibidos.
 *
 * @param rank El rank del proceso actual.
 * @param rows La cantidad de filas en la malla.
 * @param cols La cantidad de columnas en la malla.
 * @param size La cantidad de procesos en el sistema.
 * @param local_data Un mapa donde las llaves son los ranks y los valores las strings de data.
 * 
 * @return void
 */
void gossip_step(int rank, int rows, int cols, int size, map<int, string>& local_data) {
    int row = rank / cols;
    int col = rank % cols;
    char recv_buffer[200];

    for (int step = 0; step < rows - 1; ++step) {
        int send_to = ((row + 1) % rows) * cols + col;      // same col but below
        int receive_from = ((row + rows - 1) % rows) * cols + col; // above same col

        string send_buffer = concatenar(local_data);

        MPI_Request send_request, recv_request;
        MPI_Irecv(recv_buffer, 200, MPI_CHAR, receive_from, 0, MPI_COMM_WORLD, &recv_request);
        MPI_Isend(send_buffer.c_str(), send_buffer.size() + 1, MPI_CHAR, send_to, 0, MPI_COMM_WORLD, &send_request);

        MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
        MPI_Wait(&send_request, MPI_STATUS_IGNORE);

        string received_string(recv_buffer);
        int source_rank = (rank - cols + size) % size; // calculate the rank of the data sender
        local_data[source_rank] = received_string;
    }
}

/**
 * @brief Simula un proceso de broadcast inverso.
 * 
 * La funcion hace el paso de broadcast inverso donde los procesos de la diagonal envían su data a todos los procesos de su fila.
 * Los procesos que no se encuentran en la diagonal solo reciben datos de la diagonal.
 * 
 * @param rank El rank del proceso actual.
 * @param rows La cantidad de filas en la malla.
 * @param cols La cantidad de columnas en la malla.
 * @param starting_data El string de data que el proceso diagonal va a enviar.
 * @param resulting_data A map where the keys are ranks and the values are the received data.
 *
 * @return void
 */
void reverse_broadcast_step(int rank, int rows, int cols, const string& starting_data, map<int, string>& resulting_data) {
    int row = rank / cols;
    int col = rank % cols;
    char recv_buffer[200];

    if (col == row) {
        for (int c = 0; c < cols; ++c) {
            if (c != col) {
                int send_to = row * cols + c;
                MPI_Send(starting_data.c_str(), starting_data.size() + 1, MPI_CHAR, send_to, 0, MPI_COMM_WORLD);
            }
        }
        resulting_data[0] = starting_data;
    } else {
        int send_from = row * cols + row;
        MPI_Recv(recv_buffer, 200, MPI_CHAR, send_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        string received_string(recv_buffer);
        resulting_data[0] = received_string;
    }
}

string sort_and_print_by_rank(const vector<int>& aggregated_ranks, const string& result) {
    vector<pair<int, char>> rank_with_indices;

    for (size_t i = 0; i < result.size(); ++i) {
        rank_with_indices.emplace_back(aggregated_ranks[i], result[i]);
    }

    sort(rank_with_indices.begin(), rank_with_indices.end());
    string sorted_result;

    for(const auto& rank : rank_with_indices) {
        sorted_result += rank.second;
    }

    cout << "\n========================== Aggregated Ranks: ";
    copy(aggregated_ranks.begin(), aggregated_ranks.end(), ostream_iterator<int>(cout, " "));
    cout << endl;

    cout << "========================== Reordered String:" << sorted_result << endl;
    return sorted_result;
}

pair<string, vector<int>> calculate_and_print_ranks(int rank, int rows, int cols, const string& starting_data, const string& result) {
    vector<int> local_ranking = local_rank(starting_data, result);
    string sorted_result;

    cout << "\n-=-=-=-=-= Process " << rank << " (Row " << (rank / cols) << ", Col " << (rank % cols) << ") has:\n";
    cout << "Local ranks: ";
    copy(local_ranking.begin(), local_ranking.end(), ostream_iterator<int>(cout, " "));
    cout << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    int row = rank / cols;
    int col = rank % cols;
    int diagonal_process = row * cols + row;
    char recv_buffer[200];
    string recv_word;

    if (col != row) {
        MPI_Send(local_ranking.data(), local_ranking.size(), MPI_INT, diagonal_process, 0, MPI_COMM_WORLD);
        cout << "Process " << rank << " sent ranks to diagonal process " << diagonal_process << endl;
    } else {
        vector<int> aggregated_ranks(local_ranking.size(), 0);
        for (int c = 0; c < cols; ++c) {
            if (c != col) {
                vector<int> received_ranks(local_ranking.size());
                MPI_Recv(received_ranks.data(), received_ranks.size(), MPI_INT, row * cols + c, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (size_t i = 0; i < aggregated_ranks.size(); ++i) {
                    aggregated_ranks[i] += received_ranks[i];
                }
            } else {
                for (size_t i = 0; i < aggregated_ranks.size(); ++i) {
                    aggregated_ranks[i] += local_ranking[i];
                }
            }
        }

        sorted_result = result;

        if (rank != 0) {
            MPI_Send(aggregated_ranks.data(), aggregated_ranks.size(), MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(sorted_result.c_str(), sorted_result.size() + 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
            cout << "Diagonal process " << rank << " sent aggregated ranks to Process 0" << endl;
        } else {
            vector<int> global_ranks(aggregated_ranks);
            for (int r = 1; r < rows; ++r) {
                vector<int> received_ranks(aggregated_ranks.size());
                MPI_Recv(received_ranks.data(), received_ranks.size(), MPI_INT, r * cols + r, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(recv_buffer, 200, MPI_CHAR, r * cols + r, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                string received_string(recv_buffer);
                recv_word = received_string;
                cout << "RECV STRING: " << received_string << endl;
                cout << "Process 0 received aggregated ranks from diagonal process " << (r * cols + r) << endl;

                global_ranks.insert(global_ranks.end(), received_ranks.begin(), received_ranks.end());
            }

            sorted_result = sort_and_print_by_rank(global_ranks, (result + recv_word));
        }
    }

    return {sorted_result, local_ranking};
}


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    const int rows = 2;
    const int cols = 2;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    string local_string;
    vector<int> local_vector;
    pair<string, vector<int>> final_output;

    if (size != rows * cols) {
        if (rank == 0) cerr << "Error: This program requires exactly " << rows * cols << " processes." << endl;
        MPI_Finalize();
        return 1;
    }

    int row = rank / cols;
    int col = rank % cols;

    map<int, string> local_data = {
        {rank, string(1, 'a' + rank) + 
                string(1, 'a' + rank + 13) + 
                string(1, 'a' + rank + 8) + 
                string(1, 'a' + rank + 21)}
    };

    map<int, string> resulting_data;

    cout << "Process " << rank << " (Row " << row << ", Col " << col << ") starts with: " << concatenar(local_data) << endl;
    // ----------------------------------------------
    gossip_step(rank, rows, cols, size, local_data);
    // ---------------------------------------------- Local data contiene la información compartida en el gossip.
    string gossip_result = concatenar(local_data); 
    // ----------------------------------------------
    reverse_broadcast_step(rank, rows, cols, gossip_result, resulting_data);
    // ---------------------------------------------- Resulting data contiene la información compartida en el bcast.
    string result2 = concatenar(resulting_data);
    // ---------------------------------------------- Realiza el paso de sort previo al local ranking. Utiliza quick sort.
    sort(gossip_result.begin(), gossip_result.end()); 
    // ---------------------------------------------- Local ranking y Reduce
    MPI_Barrier(MPI_COMM_WORLD);

    final_output = calculate_and_print_ranks(rank, rows, cols, gossip_result, result2);

    MPI_Finalize();
    cout << endl;
    return 0;
}