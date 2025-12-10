#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <sqlite3.h>

class Database {
public:
    explicit Database(const std::string& filename);
    ~Database();

    // 统一对外暴露这两个名字，防止别的文件用旧名字
    sqlite3* getHandle() { return db_; }
    sqlite3* getDB()     { return db_; }

private:
    sqlite3* db_{nullptr};
    void initSchema();
};

#endif // DATABASE_H
