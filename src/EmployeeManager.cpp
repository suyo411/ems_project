#include "EmployeeManager.h"
#include <iostream>
#include <iomanip>
#include <limits>

EmployeeManager::EmployeeManager(Database& db)
    : db_(db) {}

void EmployeeManager::addEmployee() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    std::string name, department, position, hireDate;
    double baseSalary = 0.0;

    std::cout << "\n=== Add New Employee ===\n";
    std::cout << "Name: ";
    std::getline(std::cin, name);

    std::cout << "Department: ";
    std::getline(std::cin, department);

    std::cout << "Position: ";
    std::getline(std::cin, position);

    std::cout << "Hire date (e.g. 2025-09-18): ";
    std::getline(std::cin, hireDate);

    std::cout << "Base salary: ";
    std::cin >> baseSalary;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const char* sql =
        "INSERT INTO employee(name, department, position, hire_date, base_salary) "
        "VALUES(?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare insert employee.\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, department.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, position.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, hireDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, baseSalary);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert employee.\n";
    } else {
        std::cout << "Employee added successfully.\n";
    }

    sqlite3_finalize(stmt);
}

void EmployeeManager::listEmployees() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    std::cout << "\n=== Employee List ===\n";

    const char* sql =
        "SELECT id, name, department, position, hire_date, base_salary "
        "FROM employee "
        "ORDER BY id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare list employees.\n";
        return;
    }

    std::cout << std::left
              << std::setw(4)  << "ID"
              << std::setw(15) << "Name"
              << std::setw(12) << "Dept"
              << std::setw(20) << "Position"
              << std::setw(12) << "Hire Date"
              << "Salary"
              << "\n";

    std::cout << std::string(75, '-') << "\n";

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* dept = sqlite3_column_text(stmt, 2);
        const unsigned char* pos  = sqlite3_column_text(stmt, 3);
        const unsigned char* hdate= sqlite3_column_text(stmt, 4);
        double baseSalary         = sqlite3_column_double(stmt, 5);

        std::cout << std::left
                  << std::setw(4)  << id
                  << std::setw(15) << (name ? reinterpret_cast<const char*>(name) : "")
                  << std::setw(12) << (dept ? reinterpret_cast<const char*>(dept) : "")
                  << std::setw(20) << (pos  ? reinterpret_cast<const char*>(pos)  : "")
                  << std::setw(12) << (hdate? reinterpret_cast<const char*>(hdate): "")
                  << baseSalary
                  << "\n";
    }

    sqlite3_finalize(stmt);
}

void EmployeeManager::listEmployeesByDepartment() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    std::string dept;
    std::cout << "\n=== List Employees By Department ===\n";
    std::cout << "Enter department name (e.g. Sales): ";
    std::getline(std::cin, dept);

    if (dept.empty()) {
        std::cout << "Department name cannot be empty.\n";
        return;
    }

    const char* sql =
        "SELECT id, name, department, position, hire_date, base_salary "
        "FROM employee "
        "WHERE department = ? "
        "ORDER BY id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare list employees by department.\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, dept.c_str(), -1, SQLITE_TRANSIENT);

    std::cout << "\n=== Employees in Department: " << dept << " ===\n";
    std::cout << std::left
              << std::setw(4)  << "ID"
              << std::setw(15) << "Name"
              << std::setw(12) << "Dept"
              << std::setw(20) << "Position"
              << std::setw(12) << "Hire Date"
              << "Salary"
              << "\n";
    std::cout << std::string(75, '-') << "\n";

    int count = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ++count;

        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name  = sqlite3_column_text(stmt, 1);
        const unsigned char* deptC = sqlite3_column_text(stmt, 2);
        const unsigned char* pos   = sqlite3_column_text(stmt, 3);
        const unsigned char* hdate = sqlite3_column_text(stmt, 4);
        double baseSalary          = sqlite3_column_double(stmt, 5);

        std::cout << std::left
                  << std::setw(4)  << id
                  << std::setw(15) << (name  ? reinterpret_cast<const char*>(name)  : "")
                  << std::setw(12) << (deptC ? reinterpret_cast<const char*>(deptC) : "")
                  << std::setw(20) << (pos   ? reinterpret_cast<const char*>(pos)   : "")
                  << std::setw(12) << (hdate ? reinterpret_cast<const char*>(hdate) : "")
                  << baseSalary
                  << "\n";
    }

    if (count == 0) {
        std::cout << "No employees found in department '" << dept << "'.\n";
    } else {
        std::cout << "\nTotal employees in " << dept << ": " << count << "\n";
    }

    sqlite3_finalize(stmt);
}

void EmployeeManager::deleteEmployee() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "Database not available.\n";
        return;
    }

    int id;
    std::cout << "\n=== Delete Employee ===\n";
    std::cout << "Enter employee ID to delete: ";
    std::cin >> id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const char* sql = "DELETE FROM employee WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare delete employee.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    std::cout << "Employee deleted (if ID existed).\n";
}
