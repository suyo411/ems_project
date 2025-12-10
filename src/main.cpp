#include <iostream>
#include <limits>
#include "Database.h"
#include "EmployeeManager.h"
#include "AttendanceManager.h"
#include "SalaryManager.h"
#include "AiAssistant.h"

void showMainMenu() {
    std::cout << "\n===== Employee Management System =====\n";
    std::cout << "1. Employee Management\n";
    std::cout << "2. Attendance Management\n";
    std::cout << "3. Salary Management\n";
    std::cout << "4. AI Assistant\n";
    std::cout << "0. Exit\n";
    std::cout << "Choose: ";
}

void showEmployeeMenu() {
    std::cout << "\n--- Employee Management ---\n";
    std::cout << "1. Add employee\n";
    std::cout << "2. List employees (all)\n";
    std::cout << "3. List employees by department\n";  // 新功能
    std::cout << "4. Delete employee\n";
    std::cout << "0. Back\n";
    std::cout << "Choose: ";
}

void showAttendanceMenu() {
    std::cout << "\n--- Attendance Management ---\n";
    std::cout << "1. Add attendance record\n";
    std::cout << "2. List attendance records\n";
    std::cout << "3. Delete attendance record\n";
    std::cout << "0. Back\n";
    std::cout << "Choose: ";
}

void showSalaryMenu() {
    std::cout << "\n--- Salary Management ---\n";
    std::cout << "1. Add salary record\n";
    std::cout << "2. List salary records\n";
    std::cout << "0. Back\n";
    std::cout << "Choose: ";
}

int main() {
    Database db("database/ems.db");

    EmployeeManager employeeManager(db);
    AttendanceManager attendanceManager(db);
    SalaryManager salaryManager(db);
    AiAssistant aiAssistant(db);

    int choice = -1;

    while (true) {
        showMainMenu();
        if (!(std::cin >> choice)) {
            break;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) {
            std::cout << "Goodbye!\n";
            break;
        }
        else if (choice == 1) {
            int empChoice = -1;
            while (true) {
                showEmployeeMenu();
                if (!(std::cin >> empChoice)) break;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (empChoice == 0) {
                    break;
                } else if (empChoice == 1) {
                    employeeManager.addEmployee();
                } else if (empChoice == 2) {
                    employeeManager.listEmployees();              // 全部员工
                } else if (empChoice == 3) {
                    employeeManager.listEmployeesByDepartment();  // 按部门
                } else if (empChoice == 4) {
                    employeeManager.deleteEmployee();             // 删除
                } else {
                    std::cout << "Invalid choice.\n";
                }
            }
        }
        else if (choice == 2) {
            int attChoice = -1;
            while (true) {
                showAttendanceMenu();
                if (!(std::cin >> attChoice)) break;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (attChoice == 0) {
                    break;
                }
                else if (attChoice == 1) {
                    employeeManager.listEmployees();
                    attendanceManager.addAttendance();
                }
                else if (attChoice == 2) {
                    attendanceManager.listAttendance();
                }
                else if (attChoice == 3) {
                    attendanceManager.deleteAttendance();
                }
                else {
                    std::cout << "Invalid choice.\n";
                }
            }
        }
        else if (choice == 3) {
            int salChoice = -1;
            while (true) {
                showSalaryMenu();
                if (!(std::cin >> salChoice)) break;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (salChoice == 0) {
                    break;
                } else if (salChoice == 1) {
                    employeeManager.listEmployees();
                    salaryManager.addSalary();
                } else if (salChoice == 2) {
                    salaryManager.listSalaries();
                } else {
                    std::cout << "Invalid choice.\n";
                }
            }
        }
        else if (choice == 4) {
            aiAssistant.run();
        }
        else {
            std::cout << "Invalid choice.\n";
        }
    }

    return 0;
}
