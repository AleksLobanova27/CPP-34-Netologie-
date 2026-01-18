#include <iostream>
#include <vector>
#include <future>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace std;

template <typename Iterator>
void find_min_async(Iterator begin, Iterator end, promise<Iterator>&& prom) {
    auto min_it = min_element(begin, end);
    prom.set_value(min_it); 
}

template <typename T>
void selection_sort_async(vector<T>& arr) {
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        promise<typename vector<T>::iterator> prom;
        future<typename vector<T>::iterator> fut = prom.get_future();

        auto search_begin = it;
        auto search_end = arr.end();
        async(launch::async, find_min_async<typename vector<T>::iterator>,
            search_begin, search_end, move(prom));

        auto min_it = fut.get();

        if (min_it != it) {
            iter_swap(it, min_it);
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    vector<int> arr = { 64, 25, 12, 22, 11, 90, 3, 100, 1 };

    cout << "Изночальный массив: ";
    for (auto& x : arr) cout << x << " ";
    cout << "\n";

    selection_sort_async(arr);

    cout << "Отсортированный массив: ";
    for (auto& x : arr) cout << x << " ";
    cout << "\n";

    return 0;
}