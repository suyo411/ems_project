#include "SalaryManager.h"
#include <iostream>
#include <iomanip>
#include <limits>

SalaryManager::SalaryManager(Database& db)
    : db_(db) {}

void SalaryManager::addSalary() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    int employeeId;
    std::string month;
    double base = 0.0;
    double bonus = 0.0;
    double deduction = 0.0;

    std::cout << "\n=== Add Salary Record ===\n";
    std::cout << "Employee ID: ";
    std::cin >> employeeId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Month (e.g. 2025-12): ";
    std::getline(std::cin, month);

    std::cout << "Base salary: ";
    std::cin >> base;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Bonus: ";
    std::cin >> bonus;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Deduction: ";
    std::cin >> deduction;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const char* sql =
        "INSERT INTO salary(employee_id, month, base, bonus, deduction) "
        "VALUES(?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare insert salary.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, employeeId);
    sqlite3_bind_text(stmt, 2, month.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, base);
    sqlite3_bind_double(stmt, 4, bonus);
    sqlite3_bind_double(stmt, 5, deduction);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert salary.\n";
    } else {
        std::cout << "Salary record added successfully.\n";
    }

    sqlite3_finalize(stmt);
}

void SalaryManager::listSalaries() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    std::cout << "\n=== Salary Records ===\n";

    const char* sql =
        "SELECT s.id, e.name, s.month, s.base, s.bonus, s.deduction "
        "FROM salary s "
        "LEFT JOIN employee e ON s.employee_id = e.id "
        "ORDER BY s.month DESC, s.id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare list salaries.\n";
        return;
    }

    std::cout << std::left
              << std::setw(4)  << "ID"
              << std::setw(15) << "Name"
              << std::setw(10) << "Month"
              << std::setw(10) << "Base"
              << std::setw(10) << "Bonus"
              << std::setw(12) << "Deduction"
              << std::setw(10) << "Total"
              << "\n";

    std::cout << std::string(71, '-') << "\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name  = sqlite3_column_text(stmt, 1);
        const unsigned char* month = sqlite3_column_text(stmt, 2);
        double base      = sqlite3_column_double(stmt, 3);
        double bonus     = sqlite3_column_double(stmt, 4);
        double deduction = sqlite3_column_double(stmt, 5);
        double total     = base + bonus - deduction;

        std::cout << std::left
                  << std::setw(4)  << id
                  << std::setw(15) << (name  ? reinterpret_cast<const char*>(name)  : "")
                  << std::setw(10) << (month ? reinterpret_cast<const char*>(month) : "")
                  << std::setw(10) << base
                  << std::setw(10) << bonus
                  << std::setw(12) << deduction
                  << std::setw(10) << total
                  << "\n";
    }

    sqlite3_finalize(stmt);
}
