#include "AiAssistant.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <limits>

AiAssistant::AiAssistant(Database& db)
    : db_(db) {}

std::string AiAssistant::toLower(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return res;
}

void AiAssistant::run() {
    std::cout << "\n=== AI Assistant ===\n";
    std::cout << "This is a very simple rule-based assistant.\n";
    std::cout << "You can try questions like:\n";
    std::cout << "  - who has the highest salary\n";
    std::cout << "  - show employees in Sales\n";
    std::cout << "  - show attendance for employee 3\n";
    std::cout << "Type 'back' to return to the main menu.\n\n";

    std::string line;
    while (true) {
        std::cout << "[AI] Ask a question (or 'back'): ";
        std::getline(std::cin, line);

        if (!std::cin) {
            break;
        }

        if (line == "back" || line == "BACK") {
            std::cout << "[AI] Returning to main menu.\n";
            break;
        }

        std::string lower = toLower(line);

        if (lower.find("highest salary") != std::string::npos ||
            lower.find("top salary") != std::string::npos) {
            handleTopSalary();
        }
        else if (lower.find("employees in") != std::string::npos) {
            // 解析部门名：假设在 "in " 后面
            std::size_t pos = lower.find("in ");
            std::string dept;
            if (pos != std::string::npos) {
                dept = line.substr(pos + 3); // 用原始大小写更好看
            }
            if (dept.empty()) {
                std::cout << "[AI] I couldn't detect a department name.\n";
            } else {
                handleDepartmentQuery(dept);
            }
        }
        else if (lower.find("attendance") != std::string::npos) {
            // 从问题里找第一个数字，当作员工ID
            int empId = -1;
            std::string digits;
            for (char c : lower) {
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    digits.push_back(c);
                } else if (!digits.empty()) {
                    break;
                }
            }
            if (!digits.empty()) {
                empId = std::stoi(digits);
            }

            if (empId <= 0) {
                std::cout << "[AI] Please include an employee id number in your question.\n";
                std::cout << "     Example: show attendance for employee 3\n";
            } else {
                handleEmployeeAttendance(empId);
            }
        }
        else {
            std::cout << "[AI] Sorry, I only understand a few patterns right now:\n";
            std::cout << "     - who has the highest salary\n";
            std::cout << "     - show employees in <Department>\n";
            std::cout << "     - show attendance for employee <ID>\n";
        }

        std::cout << std::endl;
    }
}

void AiAssistant::handleDepartmentQuery(const std::string& deptInput) {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "[AI] Database not available.\n";
        return;
    }

    std::string dept = deptInput;
    // 去掉前后空格
    dept.erase(dept.begin(), std::find_if(dept.begin(), dept.end(), [](unsigned char c){ return !std::isspace(c); }));
    dept.erase(std::find_if(dept.rbegin(), dept.rend(), [](unsigned char c){ return !std::isspace(c); }).base(), dept.end());

    const char* sql =
        "SELECT id, name, department, position, hire_date, base_salary "
        "FROM employee WHERE department = ? ORDER BY id;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[AI] Failed to prepare department query.\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, dept.c_str(), -1, SQLITE_TRANSIENT);

    std::cout << "[AI] Employees in department: " << dept << "\n";
    bool any = false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        any = true;
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name  = sqlite3_column_text(stmt, 1);
        const unsigned char* dep   = sqlite3_column_text(stmt, 2);
        const unsigned char* pos   = sqlite3_column_text(stmt, 3);
        const unsigned char* hdate = sqlite3_column_text(stmt, 4);
        double baseSalary          = sqlite3_column_double(stmt, 5);

        std::cout << "  - [" << id << "] "
                  << (name  ? reinterpret_cast<const char*>(name)  : "")
                  << " | Dept: " << (dep ? reinterpret_cast<const char*>(dep) : "")
                  << " | Position: " << (pos ? reinterpret_cast<const char*>(pos) : "")
                  << " | Hire Date: " << (hdate ? reinterpret_cast<const char*>(hdate) : "")
                  << " | Base Salary: " << baseSalary
                  << "\n";
    }

    if (!any) {
        std::cout << "[AI] No employees found in that department.\n";
    }

    sqlite3_finalize(stmt);
}

void AiAssistant::handleTopSalary() {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "[AI] Database not available.\n";
        return;
    }

    const char* sql =
        "SELECT e.name, e.department, s.month, s.base, s.bonus, s.deduction "
        "FROM salary s "
        "JOIN employee e ON e.id = s.employee_id "
        "ORDER BY (s.base + s.bonus - s.deduction) DESC "
        "LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[AI] Failed to prepare top salary query.\n";
        return;
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        std::cout << "[AI] I couldn't find any salary records yet.\n";
        sqlite3_finalize(stmt);
        return;
    }

    const unsigned char* name  = sqlite3_column_text(stmt, 0);
    const unsigned char* dep   = sqlite3_column_text(stmt, 1);
    const unsigned char* month = sqlite3_column_text(stmt, 2);
    double base      = sqlite3_column_double(stmt, 3);
    double bonus     = sqlite3_column_double(stmt, 4);
    double deduction = sqlite3_column_double(stmt, 5);
    double total     = base + bonus - deduction;

    std::cout << "[AI] The employee with the highest salary is:\n";
    std::cout << "     " << (name ? reinterpret_cast<const char*>(name) : "") << "\n";
    std::cout << "     Department: " << (dep ? reinterpret_cast<const char*>(dep) : "") << "\n";
    std::cout << "     Month: " << (month ? reinterpret_cast<const char*>(month) : "") << "\n";
    std::cout << "     Base: " << base << ", Bonus: " << bonus
              << ", Deduction: " << deduction << "\n";
    std::cout << "     => Total: " << total << "\n";

    sqlite3_finalize(stmt);
}

void AiAssistant::handleEmployeeAttendance(int empId) {
    sqlite3* conn = db_.getHandle();
    if (!conn) {
        std::cerr << "[AI] Database not available.\n";
        return;
    }

    const char* sql =
        "SELECT e.name, a.date, a.status, a.note "
        "FROM attendance a "
        "JOIN employee e ON e.id = a.employee_id "
        "WHERE a.employee_id = ? "
        "ORDER BY a.date DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(conn, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[AI] Failed to prepare attendance query.\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, empId);

    bool any = false;
    std::string empName;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        any = true;
        const unsigned char* name   = sqlite3_column_text(stmt, 0);
        const unsigned char* date   = sqlite3_column_text(stmt, 1);
        const unsigned char* status = sqlite3_column_text(stmt, 2);
        const unsigned char* note   = sqlite3_column_text(stmt, 3);

        if (empName.empty() && name) {
            empName = reinterpret_cast<const char*>(name);
        }

        std::cout << "  - " << (date ? reinterpret_cast<const char*>(date) : "")
                  << " | " << (status ? reinterpret_cast<const char*>(status) : "")
                  << " | " << (note ? reinterpret_cast<const char*>(note) : "")
                  << "\n";
    }

    if (!any) {
        std::cout << "[AI] No attendance records found for employee id " << empId << ".\n";
    } else {
        std::cout << "[AI] Above are attendance records for employee "
                  << empId << " (" << empName << ").\n";
    }

    sqlite3_finalize(stmt);
}
