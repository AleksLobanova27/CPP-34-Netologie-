#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <chrono>
#include <atomic>

using namespace std;

template<typename T>
class safe_queue {
private:
    queue<T> queue_;
    mutex mutex_;
    condition_variable cond_var_;

public:
    safe_queue() = default;

    void push(T item) {
        {
            lock_guard<mutex> lock(mutex_);
            queue_.push(move(item));
        }
        cond_var_.notify_one();
    }

    T pop() {
        unique_lock<mutex> lock(mutex_);
        cond_var_.wait(lock, [this]() {
            return !queue_.empty();
            });

        T item = move(queue_.front());
        queue_.pop();
        return item;
    }

    bool try_pop(T& item) {
        lock_guard<mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        item = move(queue_.front());
        queue_.pop();
        return true;
    }

    bool empty() const {
        lock_guard<mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        lock_guard<mutex> lock(mutex_);
        return queue_.size();
    }
};

class thread_pool {
private:
    vector<thread> workers_;
    safe_queue<function<void()>> tasks_;
    atomic<bool> stop_;
    mutex queue_mutex_;

public:
    thread_pool(size_t num_threads = thread::hardware_concurrency())
        : stop_(false) {

        if (num_threads == 0) {
            num_threads = 2;
        }

        cout << "Пул потоков создан с " << num_threads << " потоками\n";

        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this]() { work(); });
        }
    }

    ~thread_pool() {
        {
            lock_guard<mutex> lock(queue_mutex_);
            stop_ = true;
        }

        for (size_t i = 0; i < workers_.size(); ++i) {
            tasks_.push([]() {});
        }

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void work() {
        while (!stop_) {
            try {
                auto task = tasks_.pop();
                if (task) {
                    task();
                }
            }
            catch (...) {
            }
        }
    }

    void submit(function<void()> task) {
        if (stop_) {
            throw runtime_error("Пул остановлен");
        }
        tasks_.push(move(task));
    }

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> future<decltype(f(args...))> {
        if (stop_) {
            throw runtime_error("Пул остановлен");
        }

        using return_type = decltype(f(args...));
        auto task = make_shared<packaged_task<return_type()>>(
            bind(forward<F>(f), forward<Args>(args)...)
        );

        future<return_type> result = task->get_future();
        tasks_.push([task]() { (*task)(); });

        return result;
    }

    void shutdown() {
        stop_ = true;
        for (size_t i = 0; i < workers_.size(); ++i) {
            tasks_.push([]() {});
        }
    }

    bool is_stopped() const {
        return stop_;
    }

    size_t get_thread_count() const {
        return workers_.size();
    }

    size_t get_pending_tasks() const {
        return tasks_.size();
    }
};

void task1() {
    this_thread::sleep_for(chrono::milliseconds(100));
    cout << "Задача 1 выполнена в потоке: " << this_thread::get_id()
        << " в " << chrono::system_clock::now().time_since_epoch().count()
        << endl;
}

void task2() {
    this_thread::sleep_for(chrono::milliseconds(200));
    cout << "Задача 2 выполнена в потоке: " << this_thread::get_id()
        << " в " << chrono::system_clock::now().time_since_epoch().count()
        << endl;
}

void task3() {
    this_thread::sleep_for(chrono::milliseconds(150));
    cout << "Задача 3 выполнена в потоке: " << this_thread::get_id()
        << " в " << chrono::system_clock::now().time_since_epoch().count()
        << endl;
}

void task4() {
    this_thread::sleep_for(chrono::milliseconds(50));
    cout << "Задача 4 выполнена в потоке: " << this_thread::get_id()
        << " в " << chrono::system_clock::now().time_since_epoch().count()
        << endl;
}

int calculate_sum(int a, int b) {
    this_thread::sleep_for(chrono::milliseconds(100));
    int result = a + b;
    cout << "Сумма: " << a << " + " << b << " = " << result
        << " в потоке: " << this_thread::get_id() << endl;
    return result;
}

void complex_task(int id) {
    this_thread::sleep_for(chrono::milliseconds(300));
    cout << "Сложная задача " << id << " выполнена в потоке: "
        << this_thread::get_id() << endl;
}

int main() {
    setlocale(LC_ALL, "Russian");

    thread_pool pool;

    cout << "Количество потоков в пуле: " << pool.get_thread_count() << "\n\n";

    cout << "Тест 1: Добавление одиночных задач\n";
    pool.submit(task1);
    pool.submit(task2);
    pool.submit(task3);
    pool.submit(task4);

    this_thread::sleep_for(chrono::seconds(1));
    cout << "\n";

    cout << "Тест 2: Использование future\n";
    auto future1 = pool.submit(calculate_sum, 10, 20);
    auto future2 = pool.submit(calculate_sum, 30, 40);
    auto future3 = pool.submit(calculate_sum, 50, 60);

    cout << "Результат 1: " << future1.get() << endl;
    cout << "Результат 2: " << future2.get() << endl;
    cout << "Результат 3: " << future3.get() << endl;

    cout << "\n";

    cout << "Тест 3: Добавление двух задач каждую секунду\n";
    for (int i = 0; i < 5; ++i) {
        cout << "\nИтерация " << i + 1 << ":\n";

        pool.submit([i]() {
            this_thread::sleep_for(chrono::milliseconds(200));
            cout << "Периодическая задача A-" << i + 1
                << " выполнена в потоке: " << this_thread::get_id() << endl;
            });

        pool.submit([i]() {
            this_thread::sleep_for(chrono::milliseconds(300));
            cout << "Периодическая задача B-" << i + 1
                << " выполнена в потоке: " << this_thread::get_id() << endl;
            });

        this_thread::sleep_for(chrono::seconds(1));
    }

    cout << "\n";

    cout << "Тест 4: Использование packaged_task\n";
    packaged_task<void()> pt1([]() {
        cout << "Packaged task 1 выполнен\n";
        });

    packaged_task<void()> pt2([]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        cout << "Packaged task 2 выполнен\n";
        });

    pool.submit([&pt1]() { pt1(); });
    pool.submit([&pt2]() { pt2(); });

    this_thread::sleep_for(chrono::milliseconds(500));

    cout << "\n";

    cout << "Тест 5: Проверка работы с очередью\n";

    for (int i = 0; i < 10; ++i) {
        pool.submit([i]() {
            this_thread::sleep_for(chrono::milliseconds(50));
            cout << "Быстрая задача " << i + 1
                << " выполнена в потоке: " << this_thread::get_id() << endl;
            });
    }

    this_thread::sleep_for(chrono::seconds(2));


    return 0;
}