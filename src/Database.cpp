#include "Database.h"
#include <iostream>

Database::Database(const std::string& filename) {
    int rc = sqlite3_open(filename.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << "\n";
        db_ = nullptr;
        return;
    }
    initSchema();
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void Database::initSchema() {
    if (!db_) return;

    const char* sql =
        // 员工表
        "CREATE TABLE IF NOT EXISTS employee ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL,"
        "  department TEXT,"
        "  position TEXT,"
        "  hire_date TEXT,"
        "  base_salary REAL"
        ");"

        // 考勤表
        "CREATE TABLE IF NOT EXISTS attendance ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  employee_id INTEGER,"
        "  date TEXT,"
        "  status TEXT,"
        "  note TEXT,"
        "  FOREIGN KEY(employee_id) REFERENCES employee(id) ON DELETE SET NULL"
        ");"

        // 薪资表
        "CREATE TABLE IF NOT EXISTS salary ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  employee_id INTEGER,"
        "  month TEXT,"
        "  base REAL,"
        "  bonus REAL,"
        "  deduction REAL,"
        "  FOREIGN KEY(employee_id) REFERENCES employee(id) ON DELETE SET NULL"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to initialize schema: "
                  << (errMsg ? errMsg : "unknown error") << "\n";
        sqlite3_free(errMsg);
    } else {
        std::cout << "Database initialized successfully!\n";
    }
}
