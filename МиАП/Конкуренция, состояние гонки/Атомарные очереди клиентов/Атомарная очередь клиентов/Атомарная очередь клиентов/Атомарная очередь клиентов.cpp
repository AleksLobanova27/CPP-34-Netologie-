#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <windows.h>
using namespace std;

class ClientQueue {
private:
    atomic<int> clientCounter;  // Атом. счетчик клиентов
    int maxClients;             // Макc. кол. клиентов
    atomic<bool> isStopped;     // Флаг остановки
    atomic<bool> maxReached;    // Флаг достижения макс.


public:
    ClientQueue(int max) : clientCounter(1), maxClients(max),
        isStopped(false), maxReached(false) {
    }

    void clientThread() {
        while (!isStopped.load(memory_order_relaxed) && !maxReached.load(memory_order_relaxed)) {
            this_thread::sleep_for(chrono::seconds(1));

            int current = clientCounter.load(memory_order_acquire);
            if (current < maxClients) {
                clientCounter.fetch_add(1, memory_order_release);

                cout << "Клиент добавлен. Всего: "
                    << clientCounter.load(memory_order_acquire)
                    << "/" << maxClients << endl;

                if (clientCounter.load(memory_order_acquire) >= maxClients) {
                    maxReached.store(true, memory_order_release);
                    cout << "Достигнут максимум клиентов!" << endl;
                }
            }
            else {
                cout << "Очередь полна (" << maxClients << ")" << endl;
                maxReached.store(true, memory_order_release);
            }
        }
    }

    // Поток операциониста.
    void tellerThread() {
        while (!isStopped.load(memory_order_relaxed)) {
            this_thread::sleep_for(chrono::seconds(2));

            int current = clientCounter.load(memory_order_acquire);
            if (current > 0) {
                clientCounter.fetch_sub(1, memory_order_release);

                cout << "Клиент обслужен. Осталось: "
                    << clientCounter.load(memory_order_acquire)
                    << "/" << maxClients << endl;

                if (clientCounter.load(memory_order_acquire) == 0 &&
                    maxReached.load(memory_order_acquire)) {
                    isStopped.store(true, memory_order_release);
                    cout << "Все клиенты обслужены!" << endl;
                }
            }
            else {
                cout << "Нет клиентов для обслуживания" << endl;

                if (maxReached.load(memory_order_acquire)) {
                    isStopped.store(true, memory_order_release);
                    cout << "Работа завершена" << endl;
                }
            }
        }
        cout << "Поток операциониста завершен." << endl;
    }
    void stop() {
        isStopped.store(true, memory_order_release);
    }
    void waitForCompletion() {
        while (!maxReached.load(memory_order_acquire) ||
            clientCounter.load(memory_order_acquire) > 0) {
            this_thread::sleep_for(chrono::seconds(1));
        }
        isStopped.store(true, memory_order_release);
    }
    int getCurrentClients() const {
        return clientCounter.load(memory_order_acquire);
    }
    int getMaxClients() const {
        return maxClients;
    }
    bool isFinished() const {
        return maxReached.load(memory_order_acquire) &&
            clientCounter.load(memory_order_acquire) == 0;
    }
};

void showAtomicOperations() {
    cout << "\n[Демонстрация атомарных операций]" << endl;

    atomic<int> counter(0);

    cout << "Начальное значение: " << counter.load() << endl;

    int old1 = counter.fetch_add(3);
    cout << "fetch_add(3): было " << old1 << ", стало " << counter.load() << endl;

    int old2 = counter.fetch_sub(1);
    cout << "fetch_sub(1): было " << old2 << ", стало " << counter.load() << endl;

    counter.store(10);
    cout << "store(10), load(): " << counter.load() << endl;
}

int main() {
    setlocale(LC_ALL, "Russian");

    showAtomicOperations();

    cout << "\n[Программа 'Очередь клиентов']" << endl;

    int maxClients;
    cout << "Введите максимальное количество клиентов: ";
    cin >> maxClients;
    cout << "Клиент добавлен. Всего: 1/" << maxClients << endl;
    ClientQueue queue(maxClients);

    thread clientThread(&ClientQueue::clientThread, &queue);
    thread tellerThread(&ClientQueue::tellerThread, &queue);

    queue.waitForCompletion();

    this_thread::sleep_for(chrono::seconds(1));

    queue.stop();

    clientThread.join();
    tellerThread.join();

    return 0;
}