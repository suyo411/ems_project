#include "AttendanceManager.h"
#include <iostream>
#include <iomanip>
#include <limits>

AttendanceManager::AttendanceManager(Database& db)
    : db_(db) {}

void AttendanceManager::addAttendance() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    int employeeId;
    std::string date;
    std::string status;
    std::string note;

    std::cout << "\n=== Add Attendance ===\n";
    std::cout << "Employee ID: ";
    std::cin >> employeeId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Date (e.g. 2025-12-03): ";
    std::getline(std::cin, date);

    std::cout << "Status (Present/Absent/Leave): ";
    std::getline(std::cin, status);

    std::cout << "Note (optional, can be empty): ";
    std::getline(std::cin, note);

    const char* sql =
        "INSERT INTO attendance(employee_id, date, status, note) "
        "VALUES(?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare insert attendance.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, employeeId);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, note.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert attendance.\n";
    } else {
        std::cout << "Attendance added successfully.\n";
    }

    sqlite3_finalize(stmt);
}

void AttendanceManager::listAttendance() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    std::cout << "\n=== Attendance Records ===\n";

    const char* sql =
        "SELECT a.id, e.name, a.date, a.status, a.note "
        "FROM attendance a "
        "LEFT JOIN employee e ON a.employee_id = e.id "
        "ORDER BY a.id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare list attendance.\n";
        return;
    }

    std::cout << std::left
              << std::setw(4)  << "ID"     // 这里的 ID 是考勤记录ID
              << std::setw(15) << "Name"
              << std::setw(12) << "Date"
              << std::setw(10) << "Status"
              << "Note"
              << "\n";

    std::cout << std::string(60, '-') << "\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name   = sqlite3_column_text(stmt, 1);
        const unsigned char* date   = sqlite3_column_text(stmt, 2);
        const unsigned char* status = sqlite3_column_text(stmt, 3);
        const unsigned char* note   = sqlite3_column_text(stmt, 4);

        std::cout << std::left
                  << std::setw(4)  << id
                  << std::setw(15) << (name   ? reinterpret_cast<const char*>(name)   : "")
                  << std::setw(12) << (date   ? reinterpret_cast<const char*>(date)   : "")
                  << std::setw(10) << (status ? reinterpret_cast<const char*>(status) : "")
                  << (note ? reinterpret_cast<const char*>(note) : "")
                  << "\n";
    }

    sqlite3_finalize(stmt);
}

void AttendanceManager::deleteAttendance() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    // 先列一下当前记录，方便你看 ID
    listAttendance();

    int attId;
    std::cout << "\n=== Delete Attendance ===\n";
    std::cout << "Enter attendance ID to delete: ";
    std::cin >> attId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const char* sql = "DELETE FROM attendance WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare delete attendance.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, attId);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to delete attendance.\n";
    } else {
        std::cout << "Attendance deleted (if ID existed).\n";
    }

    sqlite3_finalize(stmt);
}
