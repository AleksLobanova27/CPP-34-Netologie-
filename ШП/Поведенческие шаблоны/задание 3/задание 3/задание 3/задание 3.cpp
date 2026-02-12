#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <stdexcept>

using namespace std;

enum class LogType {
    Warning,
    Error,
    FatalError,
    Unknown
};

class LogMessage {
public:
    LogMessage(LogType type, const string& message)
        : type_(type), message_(message) {
    }

    LogType type() const {
        return type_;
    }

    const string& message() const {
        return message_;
    }

private:
    LogType type_;
    string message_;
};

class LogHandler {
public:
    virtual ~LogHandler() = default;

    void setNext(unique_ptr<LogHandler> next) {
        next_ = std::move(next);
    }

    void receiveMessage(const LogMessage& message) {
        if (canHandle(message)) {
            handle(message);
        }
        else if (next_) {
            next_->receiveMessage(message);
        }
        else {
            throw runtime_error("Сообщение не было обработано: " + message.message());
        }
    }

protected:
    virtual bool canHandle(const LogMessage& message) const = 0;
    virtual void handle(const LogMessage& message) = 0;

private:
    unique_ptr<LogHandler> next_;
};

class FatalErrorHandler : public LogHandler {
protected:
    bool canHandle(const LogMessage& message) const override {
        return message.type() == LogType::FatalError;
    }

    void handle(const LogMessage& message) override {
        throw runtime_error("ФАТАЛЬНАЯ ОШИБКА: " + message.message());
    }
};

class ErrorHandler : public LogHandler {
public:
    explicit ErrorHandler(const string& file_path) : file_path_(file_path) {}

protected:
    bool canHandle(const LogMessage& message) const override {
        return message.type() == LogType::Error;
    }

    void handle(const LogMessage& message) override {
        ofstream file(file_path_, ios::app);
        if (file.is_open()) {
            file << "[ОШИБКА] " << message.message() << endl;
            file.close();
        }
    }

private:
    string file_path_;
};

class WarningHandler : public LogHandler {
protected:
    bool canHandle(const LogMessage& message) const override {
        return message.type() == LogType::Warning;
    }

    void handle(const LogMessage& message) override {
        cout << "[ПРЕДУПРЕЖДЕНИЕ] " << message.message() << endl;
    }
};

class UnknownHandler : public LogHandler {
protected:
    bool canHandle(const LogMessage& message) const override {
        return message.type() == LogType::Unknown;
    }

    void handle(const LogMessage& message) override {
        throw runtime_error("Неизвестный тип сообщения: " + message.message());
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    auto fatal_handler = make_unique<FatalErrorHandler>();
    auto error_handler = make_unique<ErrorHandler>("errors.txt");
    auto warning_handler = make_unique<WarningHandler>();
    auto unknown_handler = make_unique<UnknownHandler>();

    warning_handler->setNext(std::move(error_handler));
    warning_handler->setNext(std::move(fatal_handler));
    warning_handler->setNext(std::move(unknown_handler));

    LogHandler* chain = warning_handler.get();

    cout << "=== Тестирование цепочки ответственности ===" << endl;

    LogMessage warning(LogType::Warning, "Обнаружено устаревшее API");
    LogMessage error(LogType::Error, "Не удалось открыть файл конфигурации");
    LogMessage unknown(LogType::Unknown, "Неизвестная команда");

    try {
        cout << "\nОбработка предупреждения:" << endl;
        chain->receiveMessage(warning);

        cout << "\nОбработка ошибки:" << endl;
        chain->receiveMessage(error);

        cout << "\nОбработка неизвестного сообщения:" << endl;
        chain->receiveMessage(unknown);
    }
    catch (const exception& e) {
        cerr << "Исключение: " << e.what() << endl;
    }

    try {
        cout << "\nОбработка фатальной ошибки:" << endl;
        LogMessage fatal(LogType::FatalError, "Нарушение целостности памяти");
        chain->receiveMessage(fatal);
    }
    catch (const exception& e) {
        cerr << "Исключение: " << e.what() << endl;
    }

    return 0;
}