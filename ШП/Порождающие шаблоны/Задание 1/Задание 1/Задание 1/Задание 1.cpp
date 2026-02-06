#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cassert>

using namespace std;

class SqlSelectQueryBuilder {
private:
    vector<string> columns;
    string table;
    map<string, string> whereConditions;

public:
    SqlSelectQueryBuilder& AddColumn(const string& column) {
        columns.push_back(column);
        return *this;
    }

    SqlSelectQueryBuilder& AddFrom(const string& table_name) {
        table = table_name;
        return *this;
    }

    SqlSelectQueryBuilder& AddWhere(const string& column, const string& value) {
        whereConditions[column] = value;
        return *this;
    }

    string BuildQuery() const {
        string query = "SELECT ";

        if (columns.empty()) {
            query += "*";
        }
        else {
            for (size_t i = 0; i < columns.size(); ++i) {
                query += columns[i];
                if (i != columns.size() - 1) {
                    query += ", ";
                }
            }
        }

        query += " FROM " + table;

        if (!whereConditions.empty()) {
            query += " WHERE ";
            auto it = whereConditions.begin();
            while (it != whereConditions.end()) {
                query += it->first + "=" + it->second;
                ++it;
                if (it != whereConditions.end()) {
                    query += " AND ";
                }
            }
        }

        query += ";";
        return query;
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    SqlSelectQueryBuilder query_builder;
    query_builder.AddColumn("name").AddColumn("phone");
    query_builder.AddFrom("students");
    query_builder.AddWhere("id", "42").AddWhere("name", "John");

    string query = query_builder.BuildQuery();
    cout << "Сформированный запрос: " << query << endl;

    string expected = "SELECT name, phone FROM students WHERE id=42 AND name=John;";
    assert(query == expected && "Запрос не соответствует ожидаемому!");
    cout << "Проверка пройдена успешно!" << endl;

    cout << "\nДополнительные тесты:" << endl;

    // Тест 1: Без указания колонок
    SqlSelectQueryBuilder query_builder2;
    query_builder2.AddFrom("users");
    query_builder2.AddWhere("active", "1");
    cout << "Тест 1 (SELECT *): " << query_builder2.BuildQuery() << endl;

    // Тест 2: Только таблица без условий
    SqlSelectQueryBuilder query_builder3;
    query_builder3.AddColumn("id").AddColumn("email");
    query_builder3.AddFrom("customers");
    cout << "Тест 2 (без WHERE): " << query_builder3.BuildQuery() << endl;

    // Тест 3: Перезапись таблицы
    SqlSelectQueryBuilder query_builder4;
    query_builder4.AddFrom("old_table");
    query_builder4.AddFrom("new_table"); 
    cout << "Тест 3 (перезапись таблицы): " << query_builder4.BuildQuery() << endl;

    return 0;
}