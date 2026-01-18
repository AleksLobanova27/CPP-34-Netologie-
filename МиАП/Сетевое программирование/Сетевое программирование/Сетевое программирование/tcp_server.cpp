#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

const int PORT = 8080;
const int MAX_CONNECTIONS = 10;
const int BUFFER_SIZE = 1024;

mutex cout_mutex;

void handle_client(int client_socket, int client_id) {
    char buffer[BUFFER_SIZE] = { 0 };
    const char* response = "Привет от TCP-сервера!";

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Поток " << this_thread::get_id() << " обрабатывает клиента " << client_id << endl;
    }
    
    int valread = read(client_socket, buffer, BUFFER_SIZE);

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Сообщение от клиента " << client_id << ": " << buffer << endl;
    }

    send(client_socket, response, strlen(response), 0);

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Ответ отправлен клиенту " << client_id << endl;
    }

    close(client_socket);

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Клиент " << client_id << " отключен" << endl;
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int client_counter = 0;
    vector<thread> threads;

    cout << "Запуск TCP-сервера..." << endl;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Ошибка создания сокета" << endl;
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        cerr << "Ошибка настройки сокета" << endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Ошибка привязки сокета" << endl;
        return -1;
    }

    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        cerr << "Ошибка прослушивания порта" << endl;
        return -1;
    }

    cout << "TCP-сервер прослушивает порт " << PORT << "..." << endl;
    cout << "Сервер готов принимать подключения" << endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            cerr << "Ошибка принятия подключения" << endl;
            continue;
        }

        client_counter++;

        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Новое подключение принято! ID клиента: " << client_counter << endl;
            cout << "IP клиента: " << inet_ntoa(address.sin_addr) << endl;
            cout << "Порт клиента: " << ntohs(address.sin_port) << endl;
        }

        threads.emplace_back(handle_client, new_socket, client_counter);

        for (auto it = threads.begin(); it != threads.end();) {
            if (it->joinable()) {
                it->detach();
                it = threads.erase(it);
            }
            else {
                ++it;
            }
        }

        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Активных потоков: " << threads.size() << endl;
            cout << "Всего клиентов: " << client_counter << endl;
        }
    }

    close(server_fd);

    return 0;
}