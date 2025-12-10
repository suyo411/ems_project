#ifndef AI_ASSISTANT_H
#define AI_ASSISTANT_H

#include <string>
#include "Database.h"

// 一个非常简单的规则引擎式“小AI助手”
// 支持的自然语言示例：
//  - "who has the highest salary"
//  - "show employees in Sales"
//  - "show attendance for employee 3"
//  - 输入 back 返回主菜单
class AiAssistant {
public:
    explicit AiAssistant(Database& db);

    // 进入 AI 助手对话循环
    void run();

private:
    Database& db_;

    // 处理“显示某部门员工”
    void handleDepartmentQuery(const std::string& dept);

    // 处理“最高工资是谁”
    void handleTopSalary();

    // 处理“某个员工的考勤”
    void handleEmployeeAttendance(int empId);

    // 把字符串转成小写，方便匹配关键字
    std::string toLower(const std::string& s);
};

#endif // AI_ASSISTANT_H
