#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
using namespace std;

class ClientQueue {
private:
    atomic<int> clientCounter;
    int maxClients;
    atomic<bool> isStopped;
    atomic<bool> maxReached;
    mutex mtx;
    condition_variable cv;

public:
    ClientQueue(int max) : clientCounter(1), maxClients(max), isStopped(false),
        maxReached(false) {
    }

    void clientThread() {
        while (!isStopped.load() && !maxReached.load()) {
            this_thread::sleep_for(chrono::seconds(1));

            int current = clientCounter.load();
            if (current < maxClients) {
                clientCounter.fetch_add(1);
                cout << "Клиент добавлен. Всего клиентов: "
                    << clientCounter.load() << "/" << maxClients << endl;

                if (clientCounter.load() >= maxClients) {
                    maxReached = true;
                    cout << "\nМаксимальное количество клиентов ("
                        << maxClients << ") достигнуто!\n" << endl;
                    break;
                }
            }
        }


        cv.notify_one();
    }

    void tellerThread() {
        while (!isStopped.load()) {
            this_thread::sleep_for(chrono::seconds(2));

            if (clientCounter.load() > 0) {
                clientCounter.fetch_sub(1);
                cout << "Обслужен. Осталось клиентов: "
                    << clientCounter.load() << "/" << maxClients << endl;

                if (clientCounter.load() == 0 && maxReached.load()) {
                    cout << "\nВсе клиенты обслужены!" << endl;
                    stop();
                    break;
                }
            }
            else {
                unique_lock<mutex> lock(mtx);
                if (cv.wait_for(lock, chrono::seconds(1),
                    [this]() { return clientCounter.load() > 0 || isStopped.load(); })) {
                    continue;
                }
                else {
                    if (maxReached.load() && clientCounter.load() == 0) {
                        cout << "\nБольше клиентов не будет. Завершаю работу." << endl;
                        stop();
                        break;
                    }
                    cout << "Жду клиентов..." << endl;
                }
            }
        }
        cout << "Поток завершён." << endl;
    }

    void stop() {
        isStopped = true;
        cv.notify_all();
    }

    int getCurrentClients() const {
        return clientCounter.load();
    }

    int getMaxClients() const {
        return maxClients;
    }

 
};

int main() {
    setlocale(LC_ALL, "Russian");
    int maxClients;
    cout << "Введите максимальное количество клиентов: ";
    cin >> maxClients;
    cout << "Первый клиент добавлен." << endl;
    ClientQueue queue(maxClients);

    thread clientThread(&ClientQueue::clientThread, &queue);
    thread tellerThread(&ClientQueue::tellerThread, &queue);

    clientThread.join();
    tellerThread.join();

    cout << "Максимальное количество клиентов: " << queue.getMaxClients() << endl;
    cout << "Осталось клиентов: " << queue.getCurrentClients() << endl;


    return 0;
}