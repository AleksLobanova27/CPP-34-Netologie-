#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>

using namespace std;

class LogCommand {
public:
    virtual ~LogCommand() = default;
    virtual void print(const string& message) = 0;
};

class ConsoleLogCommand : public LogCommand {
public:
    void print(const string& message) override {
        cout << "[Консоль] " << message << endl;
    }
};

class FileLogCommand : public LogCommand {
public:
    explicit FileLogCommand(const string& file_path) : file_path_(file_path) {}

    void print(const string& message) override {
        ofstream file(file_path_, ios::app);
        if (file.is_open()) {
            file << "[Файл] " << message << endl;
            file.close();
        }
        else {
            cerr << "Ошибка: Не удалось открыть файл " << file_path_ << endl;
        }
    }

private:
    string file_path_;
};

void print(LogCommand& command) {
    string test_message = "Тестовое сообщение лога";
    command.print(test_message);
}

class ComplexLogCommand : public LogCommand {
public:
    explicit ComplexLogCommand(vector<unique_ptr<LogCommand>> commands)
        : commands_(std::move(commands)) {
    }

    void print(const string& message) override {
        for (const auto& command : commands_) {
            command->print(message);
        }
    }

private:
    vector<unique_ptr<LogCommand>> commands_;
};

int main() {
    setlocale(LC_ALL, "Russian");
    ConsoleLogCommand console_logger;
    FileLogCommand file_logger("log.txt");

    cout << "Тестирование отдельных команд:" << endl;
    print(console_logger);
    print(file_logger);

    cout << "\nТестирование прямого использования:" << endl;
    console_logger.print("Прямое сообщение в консоль");
    file_logger.print("Прямое сообщение в файл");

    cout << "\nТестирование комплексной команды:" << endl;
    vector<unique_ptr<LogCommand>> commands;
    commands.push_back(make_unique<ConsoleLogCommand>());
    commands.push_back(make_unique<FileLogCommand>("complex_log.txt"));

    ComplexLogCommand complex_logger(std::move(commands));
    complex_logger.print("Сообщение одновременно в консоль и файл");

    cout << "\nТестирование с разными сообщениями:" << endl;
    ConsoleLogCommand console;
    console.print("Приложение запущено");
    console.print("Обработка данных...");
    console.print("Приложение завершено");

    return 0;
}