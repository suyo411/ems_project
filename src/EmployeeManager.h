#ifndef EMPLOYEE_MANAGER_H
#define EMPLOYEE_MANAGER_H

#include "Database.h"

class EmployeeManager {
public:
    explicit EmployeeManager(Database& db);

    void addEmployee();                 // 添加员工
    void listEmployees();               // 列出全部员工
    void listEmployeesByDepartment();   // 按部门列出员工（新加功能）
    void deleteEmployee();              // 删除员工

private:
    Database& db_;
};

#endif // EMPLOYEE_MANAGER_H
