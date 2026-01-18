#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

class Data {
private:
    int value;
    mutex mtx;

public:
    Data(int val) : value(val) {}

    int getValue() const {
        return value;
    }

    void setValue(int val) {
        value = val;
    }

    mutex& getMutex() {
        return mtx;
    }

    void swapValues(Data& other) {
        swap(value, other.value);
    }
};

// Вариант 1: через lock() с adopt_lock (ваш исходный вариант - правильный)
void swap1(Data& a, Data& b) {
    lock(a.getMutex(), b.getMutex());
    lock_guard<mutex> lockA(a.getMutex(), adopt_lock);
    lock_guard<mutex> lockB(b.getMutex(), adopt_lock);
    a.swapValues(b);
}

// Вариант 2: через scoped_lock (C++17) - добавляем по заданию
void swap2(Data& a, Data& b) {
    scoped_lock lockBoth(a.getMutex(), b.getMutex());
    a.swapValues(b);
}

// Вариант 3: через unique_lock с defer_lock (ваш исходный вариант - правильный)
void swap3(Data& a, Data& b) {
    unique_lock<mutex> lockA(a.getMutex(), defer_lock);
    unique_lock<mutex> lockB(b.getMutex(), defer_lock);
    lock(lockA, lockB);
    a.swapValues(b);
}

int main() {
    setlocale(LC_ALL, "Russian");
    Data data1(10);
    Data data2(20);

    cout << "Перед обменом:\n";
    cout << "data1 = " << data1.getValue() << "\n";
    cout << "data2 = " << data2.getValue() << "\n";

    // Тестируем swap1 (lock + lock_guard)
    swap1(data1, data2);
    cout << "\nПосле swap1 (lock + lock_guard):\n";
    cout << "data1 = " << data1.getValue() << "\n";
    cout << "data2 = " << data2.getValue() << "\n";

    // Возвращаем исходные значения
    swap1(data1, data2);

    // Тестируем swap2 (scoped_lock)
    swap2(data1, data2);
    cout << "\nПосле swap2 (scoped_lock):\n";
    cout << "data1 = " << data1.getValue() << "\n";
    cout << "data2 = " << data2.getValue() << "\n";

    // Возвращаем исходные значения
    swap2(data1, data2);

    // Тестируем swap3 (unique_lock)
    swap3(data1, data2);
    cout << "\nПосле swap3 (unique_lock):\n";
    cout << "data1 = " << data1.getValue() << "\n";
    cout << "data2 = " << data2.getValue() << "\n";

    return 0;
}