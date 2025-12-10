




    message(FATAL_ERROR "sqlite3 library not found")
    src/AiAssistant.cpp
    src/AttendanceManager.cpp
    src/Database.cpp
    src/EmployeeManager.cpp
    src/main.cpp
    src/SalaryManager.cpp
)
add_executable(ems_project
cmake_minimum_required(VERSION 3.20)
endif()
find_library(SQLITE3_LIB sqlite3)
if (NOT SQLITE3_LIB)
project(ems_project)
set(CMAKE_CXX_STANDARD 17)
target_include_directories(ems_project PRIVATE src)
target_link_libraries(ems_project PRIVATE ${SQLITE3_LIB})
