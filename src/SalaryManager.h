#ifndef SALARY_MANAGER_H
#define SALARY_MANAGER_H

#include "Database.h"

class SalaryManager {
public:
    explicit SalaryManager(Database& db);

    // 记录薪资
    void addSalary();

    // 查看薪资记录
    void listSalaries();

private:
    Database& db_;
};

#endif // SALARY_MANAGER_H
