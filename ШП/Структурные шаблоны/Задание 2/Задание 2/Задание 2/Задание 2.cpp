#include <iostream>
#include <string>
#include <memory>

using namespace std;

// Оригинальный класс из лекции
class VeryHeavyDatabase {
public:
    string GetData(const string& key) const noexcept {
        return "value";
    }
};

// Заместитель с ограничением количества обращений
class OneShotDB {
private:
    VeryHeavyDatabase* real_object;
    size_t shots_left;

public:
    explicit OneShotDB(VeryHeavyDatabase* real_object, size_t shots = 1)
        : real_object(real_object), shots_left(shots) {
    }

    string GetData(const string& key) {
        if (shots_left > 0) {
            shots_left--;
            return real_object->GetData(key);
        }
        return "error";
    }
};

int main() {
    auto real_db = VeryHeavyDatabase();
    auto limit_db = OneShotDB(addressof(real_db), 2);

    cout << limit_db.GetData("key") << endl;
    cout << limit_db.GetData("key") << endl;
    cout << limit_db.GetData("key") << endl;

    return 0;
}