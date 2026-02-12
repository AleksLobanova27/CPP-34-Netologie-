#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class Observer {
public:
    virtual ~Observer() = default;
    virtual void onWarning(const string& message) {}
    virtual void onError(const string& message) {}
    virtual void onFatalError(const string& message) {}
};

class Observable {
public:
    void addObserver(Observer* observer) {
        observers_.push_back(observer);
    }

    void removeObserver(Observer* observer) {
        auto it = remove(observers_.begin(), observers_.end(), observer);
        observers_.erase(it, observers_.end());
    }

    void warning(const string& message) const {
        for (auto observer : observers_) {
            observer->onWarning(message);
        }
    }

    void error(const string& message) const {
        for (auto observer : observers_) {
            observer->onError(message);
        }
    }

    void fatalError(const string& message) const {
        for (auto observer : observers_) {
            observer->onFatalError(message);
        }
    }

private:
    vector<Observer*> observers_;
};

class WarningObserver : public Observer {
public:
    void onWarning(const string& message) override {
        cout << "[Предупреждение] " << message << endl;
    }
};

class ErrorObserver : public Observer {
public:
    explicit ErrorObserver(const string& file_path) : file_path_(file_path) {}

    void onError(const string& message) override {
        ofstream file(file_path_, ios::app);
        if (file.is_open()) {
            file << "[Ошибка] " << message << endl;
            file.close();
        }
    }

private:
    string file_path_;
};

class FatalErrorObserver : public Observer {
public:
    explicit FatalErrorObserver(const string& file_path) : file_path_(file_path) {}

    void onFatalError(const string& message) override {
        cout << "[Фатальная ошибка] " << message << endl;

        ofstream file(file_path_, ios::app);
        if (file.is_open()) {
            file << "[Фатальная ошибка] " << message << endl;
            file.close();
        }
    }

private:
    string file_path_;
};

int main() {
    setlocale(LC_ALL, "Russian");
    Observable logger;

    WarningObserver warning_observer;
    ErrorObserver error_observer("errors.txt");
    FatalErrorObserver fatal_observer("fatal_errors.txt");

    logger.addObserver(&warning_observer);
    logger.addObserver(&error_observer);
    logger.addObserver(&fatal_observer);

    cout << "Тестирование системы логирования:" << endl;

    logger.warning("Обнаружено устаревшее API");
    logger.error("Не удалось открыть файл конфигурации");
    logger.fatalError("Нарушение целостности памяти");

    cout << "\nПосле удаления наблюдателя предупреждений:" << endl;
    logger.removeObserver(&warning_observer);

    logger.warning("Это предупреждение никто не увидит");
    logger.error("Ошибка подключения к базе данных");
    logger.fatalError("Критический сбой системы");

    return 0;
}