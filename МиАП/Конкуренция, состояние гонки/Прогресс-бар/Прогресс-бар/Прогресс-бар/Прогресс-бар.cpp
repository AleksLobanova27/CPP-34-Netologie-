#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <random>
#include <mutex>
#include <iomanip>
#include <windows.h>

using namespace std;

HANDLE hConsole;
mutex consoleMutex;

void setCursor(int x, int y) {
    lock_guard<mutex> lock(consoleMutex);
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(hConsole, coord);
}

void clearLine(int y) {
    lock_guard<mutex> lock(consoleMutex);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    COORD startPos = { 0, static_cast<SHORT>(y) };
    DWORD written;
    FillConsoleOutputCharacter(hConsole, ' ', csbi.dwSize.X, startPos, &written);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, csbi.dwSize.X, startPos, &written);
}

class ProgressBar {
private:
    int id;
    int total;
    atomic<int> progress;
    atomic<bool> done;
    int row;
    mt19937 rng;
    chrono::steady_clock::time_point start;

public:
    ProgressBar(int id, int total, int row)
        : id(id), total(total), progress(0), done(false), row(row),
        rng(random_device{}() + id) {
    }

    void run() {
        start = chrono::steady_clock::now();
        uniform_real_distribution<double> dist(0.0, 1.0);

        for (int i = 0; i < total; i++) {
            this_thread::sleep_for(chrono::milliseconds(50 + rng() % 100));

            bool error = dist(rng) < 0.10;
            progress++;
            show(error);
        }

        done = true;
        showFinal();
    }

    void show(bool error) {
        setCursor(0, row);
        clearLine(row);

        int cur = progress.load();
        int percent = (cur * 100) / total;

        // Номер потока
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << setw(2) << id << " ";

        // Прогресс-бар
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << "[";

        int barWidth = 30;
        int pos = (cur * barWidth) / total;

        for (int i = 0; i < barWidth; i++) {
            if (i < pos) cout << "|";
            else if (i == pos) {
                if (error) {
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
                    cout << "X";
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                }
                else {
                    cout << ">";
                }
            }
            else cout << ".";
        }

        cout << "] ";

        // Процент
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        cout << setw(3) << percent << "%";

        // Счетчик
        cout << " (" << setw(3) << cur << "/" << setw(3) << total << ")";

        if (error) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            cout << " !";
        }

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << flush;
    }

    void showFinal() {
        setCursor(0, row);
        clearLine(row);

        auto end = chrono::steady_clock::now();
        auto time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << setw(2) << id << " [";

        for (int i = 0; i < 30; i++) cout << "|";

        cout << "] 100% (" << setw(3) << total << "/" << setw(3) << total
            << ") " << setw(5) << time << " мс";

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        cout << flush;
    }

    bool finished() const {
        return done.load();
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");

    cout << "ПРОГРЕСС-БАР" << endl << endl;

    int n, length;
    cout << "Количество потоков: ";
    cin >> n;
    cout << "Длина расчета: ";
    cin >> length;

    if (n <= 0 || length <= 0) {
        cout << "Ошибка!" << endl;
        return 1;
    }

    system("cls");
    cout << "ПРОГРЕСС-БАР\n";

    vector<ProgressBar*> bars;
    vector<thread> threads;

    for (int i = 0; i < n; i++) {
        bars.push_back(new ProgressBar(i, length, i + 3));
        threads.emplace_back([&bars, i]() { bars[i]->run(); });
    }

    for (auto& t : threads) t.join();

    setCursor(0, n + 4);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    cout << "\nВСЕ ПОТОКИ ЗАВЕРШЕНЫ ";
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    for (auto bar : bars) delete bar;

    return 0;
}