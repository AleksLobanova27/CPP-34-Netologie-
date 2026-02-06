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

    SqlSelectQueryBuilder& AddColumns(const vector<string>& columns) noexcept {
        for (const auto& column : columns) {
            this->columns.push_back(column);
        }
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

    SqlSelectQueryBuilder& AddWhere(const map<string, string>& kv) noexcept {
        for (const auto& pair : kv) {
            whereConditions[pair.first] = pair.second;
        }
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

    cout << "Тест новых методов" << endl;

    SqlSelectQueryBuilder builder1;
    vector<string> cols = { "id", "name", "email" };
    builder1.AddColumns(cols);
    builder1.AddFrom("users");

    map<string, string> where_map = {
        {"active", "1"},
        {"deleted", "0"},
        {"verified", "1"}
    };
    builder1.AddWhere(where_map);

    cout << "Тест 1 (AddColumns + AddWhere map): " << builder1.BuildQuery() << endl;

    SqlSelectQueryBuilder builder2;
    builder2.AddColumn("title").AddColumn("author");
    vector<string> more_cols = { "year", "price" };
    builder2.AddColumns(more_cols);
    builder2.AddFrom("books");

    map<string, string> conditions;
    conditions["category"] = "fiction";
    conditions["available"] = "yes";
    builder2.AddWhere(conditions);
    builder2.AddWhere("rating", ">4.5");

    cout << "Тест 2 (комбинированный): " << builder2.BuildQuery() << endl;

    SqlSelectQueryBuilder builder3;
    builder3.AddColumns({ "*" });
    builder3.AddFrom("orders");

    map<string, string> order_conditions;
    order_conditions["status"] = "shipped";
    order_conditions["payment"] = "completed";
    builder3.AddWhere(order_conditions);

    cout << "Тест 3 (только map условия): " << builder3.BuildQuery() << endl;

    SqlSelectQueryBuilder builder4;
    vector<string> empty_cols;
    builder4.AddColumns(empty_cols);
    builder4.AddFrom("products");

    map<string, string> empty_map;
    builder4.AddWhere(empty_map);

    cout << "Тест 4 (пустые параметры): " << builder4.BuildQuery() << endl;

    cout << "\nПроверка обратной совместимости" << endl;

    SqlSelectQueryBuilder builder5;
    builder5.AddColumn("name").AddColumn("phone");
    builder5.AddFrom("students");
    builder5.AddWhere("id", "42").AddWhere("name", "John");

    string query = builder5.BuildQuery();
    cout << "Оригинальный пример: " << query << endl;

    string expected = "SELECT name, phone FROM students WHERE id=42 AND name=John;";
    assert(query == expected && "Ошибка обратной совместимости!");
    cout << "✓ Обратная совместимость подтверждена" << endl;

    return 0;
}