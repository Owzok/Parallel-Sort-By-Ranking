#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <iostream>
#include <vector>
#include <string>
using namespace std;

vector<int> local_rank(const string& local_A, const string& A) {
    vector<int> rank_counts(A.size(), 0);

    for (size_t i = 0; i < A.size(); i++) {
        for (size_t j = 0; j < local_A.size(); j++) {
            if (local_A[j] < A[i]) {
                rank_counts[i]++;
            }
        }
    }

    return rank_counts;
}

vector<int> local_ranking(const vector<string>& local_A, const vector<string>& A, int local_n, int n) {
    vector<int> local_M(local_n, 0);

    for (int i = 0; i < local_n; ++i) {
        for (int j = 0; j < n; ++j) {
            vector<char> chars_local(local_A[i].begin(), local_A[i].end());
            vector<char> chars_global(A[j].begin(), A[j].end());

            if (chars_local >= chars_global) {
                local_M[i]++;
            }
        }
    }

    return local_M; 
}

vector<int> local_ranking(const vector<string>& local_A, const vector<string>& A, int local_n, int n);

int main() {
    string s = "cgjlmn";
    string s2 = "khedbf";
    vector<int> ranks = local_rank(s, s2);
    for(auto const x: ranks){
        cout << x << " ";
    }
    cout << endl;

    return 0;
}