#ifndef ATTENDANCE_MANAGER_H
#define ATTENDANCE_MANAGER_H

#include "Database.h"

class AttendanceManager {
public:
    explicit AttendanceManager(Database& db);

    // 记录考勤
    void addAttendance();

    // 查看考勤记录
    void listAttendance();

    // 删除一条考勤记录（按考勤ID）
    void deleteAttendance();

private:
    Database& db_;
};

#endif // ATTENDANCE_MANAGER_H
