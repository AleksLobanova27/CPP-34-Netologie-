#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

const int PORT = 8080;
const char* SERVER_IP = "127.0.0.1";
const int BUFFER_SIZE = 1024;

int main() {
    setlocale(LC_ALL, "Russian");
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = { 0 };
    const char* test_message = "Привет от TCP-клиент!";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Ошибка создания сокета." << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        cerr << "Неверный адрес / Адрес не поддерживается." << endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Не удалось установить соединение" << endl;
        return -1;
    }

    cout << "Подключен к серверу по адресу " << SERVER_IP << ":" << PORT << endl;

    send(sock, test_message, strlen(test_message), 0);
    cout << "Отправленное сообщение: " << test_message << endl;

    int valread = read(sock, buffer, BUFFER_SIZE);
    cout << "Ответ сервера: " << buffer << endl;

    close(sock);

    return 0;
}