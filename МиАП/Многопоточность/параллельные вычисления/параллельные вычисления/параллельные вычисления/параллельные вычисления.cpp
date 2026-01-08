#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <random>
using namespace std;

void vector_add_part(const vector<int>& a, const vector<int>& b,
    vector<int>& result, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        result[i] = a[i] + b[i];
    }
}

double parallel_vector_add(const vector<int>& a, const vector<int>& b,
    vector<int>& result, int num_threads) {
    size_t n = a.size();
    vector<thread> threads;
    size_t chunk_size = n / num_threads;

    auto start = chrono::steady_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        size_t start_idx = i * chunk_size;
        size_t end_idx = (i == num_threads - 1) ? n : (i + 1) * chunk_size;

        threads.emplace_back(vector_add_part, ref(a), ref(b),
            ref(result), start_idx, end_idx);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;

    return elapsed.count();
}

double sequential_vector_add(const vector<int>& a, const vector<int>& b,
    vector<int>& result) {
    auto start = std::chrono::steady_clock::now();

    for (size_t i = 0; i < a.size(); ++i) {
        result[i] = a[i] + b[i];
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;

    return elapsed.count();
}

vector<int> generate_random_vector(size_t size, int min_val = 0, int max_val = 100) {
    vector<int> vec(size);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(min_val, max_val);

    for (size_t i = 0; i < size; ++i) {
        vec[i] = distrib(gen);
    }

    return vec;
}

bool check_results(const vector<int>& result_seq, const vector<int>& result_par) {
    if (result_seq.size() != result_par.size()) return false;

    for (size_t i = 0; i < result_seq.size(); ++i) {
        if (result_seq[i] != result_par[i]) {
            return false;
        }
    }

    return true;
}

int main() {
    setlocale(LC_ALL, "Russian");
    unsigned int hardware_cores = std::thread::hardware_concurrency();
    cout << "Доступное количество аппаратных ядер: " << hardware_cores << "\n\n";

    vector<size_t> sizes = { 1000, 10000, 100000, 1000000 };
    vector<int> thread_counts = { 1, 2, 4, 8, 16 };
    vector<vector<double>> results(
        thread_counts.size(),
        vector<double>(sizes.size(), 0.0)
    );

    cout << setw(10) << "Потоки\\Размер";
    for (size_t size : sizes) {
        cout << setw(12) << size;
    }
    cout << "\n";

    for (size_t thread_idx = 0; thread_idx < thread_counts.size(); ++thread_idx) {
        int num_threads = thread_counts[thread_idx];

        cout << setw(10) << num_threads << " потоков";

        for (size_t size_idx = 0; size_idx < sizes.size(); ++size_idx) {
            size_t size = sizes[size_idx];

            auto vec1 = generate_random_vector(size);
            auto vec2 = generate_random_vector(size);

            vector<int> result_seq(size);
            vector<int> result_par(size);

            double time_taken;
            if (num_threads == 1) {
                time_taken = sequential_vector_add(vec1, vec2, result_seq);
                result_par = result_seq;
            }
            else {
                time_taken = parallel_vector_add(vec1, vec2, result_par, num_threads);

                sequential_vector_add(vec1, vec2, result_seq);
                if (!check_results(result_seq, result_par)) {
                    cerr << "\nОшибка: результаты не совпадают для размера "
                        << size << " и " << num_threads << " потоков!\n";
                }
            }

            results[thread_idx][size_idx] = time_taken;
            cout << setw(12) << fixed << setprecision(7)
                << time_taken << "s";

            vector<int>().swap(result_seq);
            vector<int>().swap(result_par);
        }
        cout << "\n";
    }

    cout << "\nОптимальное количество потоков для каждого размера:\n";
    for (size_t size_idx = 0; size_idx < sizes.size(); ++size_idx) {
        double min_time = results[0][size_idx];
        int optimal_threads = 1;

        for (size_t thread_idx = 1; thread_idx < thread_counts.size(); ++thread_idx) {
            if (results[thread_idx][size_idx] < min_time) {
                min_time = results[thread_idx][size_idx];
                optimal_threads = thread_counts[thread_idx];
            }
        }

        cout << "Размер " << std::setw(7) << sizes[size_idx]
            << ": " << optimal_threads << " потоков (время: "
            << fixed << setprecision(7) << min_time << "s)\n";
    }


    return 0;
}