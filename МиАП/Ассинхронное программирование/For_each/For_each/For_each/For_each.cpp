#include <iostream>
#include <vector>
#include <future>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace std;

template <typename Iterator, typename Func>
void parallel_for_each(Iterator begin, Iterator end, Func func, size_t min_block_size = 1000) {
    size_t size = distance(begin, end);

    if (size < min_block_size) {
        for_each(begin, end, func);
        return;
    }

    Iterator mid = begin;
    advance(mid, size / 2);

    auto left_future = async(launch::async,
        parallel_for_each<Iterator, Func>,
        begin, mid, func, min_block_size);

    parallel_for_each(mid, end, func, min_block_size);

    left_future.wait();
}

int main() {
    setlocale(LC_ALL, "Russian");
    vector<int> vec(10000);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = i;
    }

    auto multiply_by_two = [](int& x) {
        x *= 2;
        };

    parallel_for_each(vec.begin(), vec.end(), multiply_by_two);

    cout << "Первые 10 элементов после parallel_for_each: ";
    for (int i = 0; i < 10; ++i) {
        cout << vec[i] << " ";
    }
    cout << "\n";

    return 0;
}