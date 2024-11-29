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

/**
 * @brief Sorts the characters in the result string based on their ranks and prints the reordered string.
 *
 * This function takes a vector of aggregated ranks and a result string as input.
 * It determines the final order of characters in the result string based on their ranks.
 * The characters are then sorted in ascending order of their ranks and printed to the console.
 *
 * @param aggregated_ranks A vector of integers representing the ranks of each character in the result string.
 * @param result A string containing the characters to be reordered.
 *
 * @return void
 */
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

    //cout << "\n========================== Aggregated Ranks: ";
    //copy(aggregated_ranks.begin(), aggregated_ranks.end(), ostream_iterator<int>(cout, " "));
    //cout << endl;

    //cout << "========================== Reordered String:" << sorted_result << endl;
    return sorted_result;
}

/**
 * @brief Calculates and prints the ranks of characters in a given string.
 *
 * This function calculates the ranks of characters in a given string based on their order in the original string.
 * It then prints the ranks, the original string, and the final result string.
 * The ranks are calculated using the local_rank function and aggregated across processes using MPI communication.
 *
 * @param rank The rank of the current process.
 * @param rows The number of rows in the process grid.
 * @param cols The number of columns in the process grid.
 * @param starting_data The initial string of characters for the current process.
 * @param result The final string of characters after all processes have completed their calculations.
 *
 * @return void
 */
string calculate_and_print_ranks(int rank, int rows, int cols, const string& starting_data, const string& result) {
    vector<int> local_ranking = local_rank(starting_data, result);
    string sorted_result;

    //cout << "\n-=-=-=-=-= Process " << rank << " Local ranks: ";
    //copy(local_ranking.begin(), local_ranking.end(), ostream_iterator<int>(cout, " "));
    //cout << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    int row = rank / cols;
    int col = rank % cols;
    int diagonal_process = row * cols + row;
    char recv_buffer[200];
    string recv_word;

    // Enviar los datos a las diagonales
    if (col != row) 
    {
        MPI_Send(local_ranking.data(), local_ranking.size(), MPI_INT, diagonal_process, 0, MPI_COMM_WORLD);
        cout << "Process " << rank << " sent ranks to diagonal process " << diagonal_process << endl;
    } 
    else 
    {
        // Los ranks de la diagonal
        vector<int> aggregated_ranks(local_ranking.size(), 0);
        for (int c = 0; c < cols; ++c) 
        {
            // recibir desde la diagonal por cada uno y sumarlo al proceso principal
            if (c != col) 
            {
                vector<int> received_ranks(local_ranking.size());
                MPI_Recv(received_ranks.data(), received_ranks.size(), MPI_INT, row * cols + c, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (size_t i = 0; i < aggregated_ranks.size(); ++i) 
                {
                    aggregated_ranks[i] += received_ranks[i];
                }
            } 
            else 
            {   // si es el diagonal, se le suma igual
                for (size_t i = 0; i < aggregated_ranks.size(); ++i) 
                {
                    aggregated_ranks[i] += local_ranking[i];
                }
            }
        }
        
        // Después cada diagonal envía su ranking y string al proceso 0
        if (rank != 0) 
        {
            MPI_Send(aggregated_ranks.data(), aggregated_ranks.size(), MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(result.c_str(), result.size() + 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
            cout << "Diagonal process " << rank << " sent aggregated ranks to Process 0" << endl;
        } 
        else 
        {
            vector<int> global_ranks(aggregated_ranks);

            for (int r = 1; r < rows; ++r) 
            {
                int d_proc = r * cols + r; // proceso diagonal en base al iterador r
                vector<int> received_ranks(aggregated_ranks.size());
                // Recibe la info de cada diagonal
                MPI_Recv(received_ranks.data(), received_ranks.size(), MPI_INT, d_proc, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(recv_buffer, 200, MPI_CHAR, d_proc, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // Tranforma la información del buffer en una string
                string received_string(recv_buffer);
                recv_word += received_string;

                cout << "Process 0 received ranks from diagonal process " << d_proc << endl;
                // Expande global ranks
                global_ranks.insert(global_ranks.end(), received_ranks.begin(), received_ranks.end());
            }
            // Usa global ranks y 
            sorted_result = sort_and_print_by_rank(global_ranks, (result + recv_word));
        }
    }
    return sorted_result;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();

    string input = "asiuydfgaheiugheao";

    int rank, size;
    const int rows = 3;
    const int cols = 3;

    if (input.size() % (rows * cols) != 0){
        if (rank == 0) cerr << "Input Size [" << input.size() << "] doesn't match row * col size [" << rows * cols << "]" << endl;
        MPI_Finalize();
        return 1;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    string local_string;
    vector<int> local_vector;
    string final_output;

    // Al trabajar con una diagonal, lo óptimo es rows == cols, y necesitamos un proceso para cada elemento de la matriz.
    if (size != rows * cols) {
        if (rank == 0) cerr << "Error: This program requires exactly " << rows * cols << " processes." << endl;
        MPI_Finalize();
        return 1;
    }

    int row = rank / cols;
    int col = rank % cols;

    int amount_data = input.size()/rank;

    map<int, string> local_data = {{rank, input.substr(rank*(amount_data), (amount_data))}};

    map<int, string> resulting_data;

    cout << "Process " << rank << " (Row " << row << ", Col " << col << ") starts with: " << concatenar(local_data) << endl;
    // ----------------------------------------------
    double gossip_time = MPI_Wtime();
    gossip_step(rank, rows, cols, size, local_data);
    // ---------------------------------------------- Local data contiene la información compartida en el gossip.
    string gossip_result = concatenar(local_data); 
    if (rank == 0) cout << "Tiempo de ejecución (Gossip): " << gossip_time - start_time << " segundos" << endl;
    // ----------------------------------------------
    double bcast_time = MPI_Wtime();
    reverse_broadcast_step(rank, rows, cols, gossip_result, resulting_data);
    // ---------------------------------------------- Resulting data contiene la información compartida en el bcast.
    string result2 = concatenar(resulting_data);
    if (rank == 0) cout << "Tiempo de ejecución (BCast): " << bcast_time - gossip_time << " segundos" << endl;
    // ---------------------------------------------- Realiza el paso de sort previo al local ranking. Utiliza quick sort.
    double sort_time = MPI_Wtime();
    sort(gossip_result.begin(), gossip_result.end()); 

    if (rank == 0) cout << "Tiempo de ejecución (Sort): " << sort_time - bcast_time << " segundos" << endl;
    // ---------------------------------------------- Local ranking y Reduce
    final_output = calculate_and_print_ranks(rank, rows, cols, gossip_result, result2);
    
    if (rank == 0) cout << "Tiempo de ejecución (Reduce): " << MPI_Wtime() - sort_time << " segundos" << endl;

    if(rank == 0) cout << "Result: " << final_output << endl;

    double end_time = MPI_Wtime();
    if (rank == 0) cout << "Tiempo de ejecución: " << end_time - start_time << " segundos" << endl;

    MPI_Finalize();

    cout << endl;

    return 0;
}