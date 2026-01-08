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

// Вариант 1
void swap1(Data& a, Data& b) {
    lock(a.getMutex(), b.getMutex());
    lock_guard<mutex> lockA(a.getMutex(), adopt_lock);
    lock_guard<mutex> lockB(b.getMutex(), adopt_lock);
    a.swapValues(b);
}

// Вариант 2
void swap2(Data& a, Data& b) {
    lock_guard<mutex> lockFirst(a.getMutex());
    lock_guard<mutex> lockSecond(b.getMutex());
    a.swapValues(b);
}

// Вариант 3
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

    // Тестируем swap1
    swap1(data1, data2);
    cout << "\nПосле swap1 (lock + lock_guard):\n";
    cout << "data1 = " << data1.getValue() << "\n";
    cout << "data2 = " << data2.getValue() << "\n";

    swap1(data1, data2);

    // Тестируем swap2
    swap2(data1, data2);
    cout << "\nПосле swap2 (вложенные lock_guard):\n";
    cout << "data1 = " << data1.getValue() << "\n";
    cout << "data2 = " << data2.getValue() << "\n";

    swap2(data1, data2);

    // Тестируем swap3
    swap3(data1, data2);
    cout << "\nПосле swap3 (unique_lock):\n";
    cout << "data1 = " << data1.getValue() << "\n";
    cout << "data2 = " << data2.getValue() << "\n";

    return 0;
}