-- 2024-09-02 的考勤（所有员工各一条）
INSERT INTO attendance (employee_id, date, status, note)
SELECT
    id AS employee_id,
    '2024-09-02' AS date,
    CASE
        WHEN id % 5 = 0 THEN 'Present'
        WHEN id % 5 = 1 THEN 'Present'
        WHEN id % 5 = 2 THEN 'Late'
        WHEN id % 5 = 3 THEN 'Remote'
        ELSE 'Absent'
    END AS status,
    'Auto-generated attendance record' AS note
FROM employee;

-- 2024-10-01 的考勤
INSERT INTO attendance (employee_id, date, status, note)
SELECT
    id AS employee_id,
    '2024-10-01' AS date,
    CASE
        WHEN id % 5 = 0 THEN 'Present'
        WHEN id % 5 = 1 THEN 'Late'
        WHEN id % 5 = 2 THEN 'Present'
        WHEN id % 5 = 3 THEN 'Present'
        ELSE 'Remote'
    END AS status,
    'Auto-generated attendance record' AS note
FROM employee;

-- 2024-11-04 的考勤
INSERT INTO attendance (employee_id, date, status, note)
SELECT
    id AS employee_id,
    '2024-11-04' AS date,
    CASE
        WHEN id % 5 = 0 THEN 'Present'
        WHEN id % 5 = 1 THEN 'Present'
        WHEN id % 5 = 2 THEN 'Absent'
        WHEN id % 5 = 3 THEN 'Remote'
        ELSE 'Present'
    END AS status,
    'Auto-generated attendance record' AS note
FROM employee;

-- 2024-12-02 的考勤
INSERT INTO attendance (employee_id, date, status, note)
SELECT
    id AS employee_id,
    '2024-12-02' AS date,
    CASE
        WHEN id % 5 = 0 THEN 'Present'
        WHEN id % 5 = 1 THEN 'Remote'
        WHEN id % 5 = 2 THEN 'Present'
        WHEN id % 5 = 3 THEN 'Late'
        ELSE 'Present'
    END AS status,
    'Auto-generated attendance record' AS note
FROM employee;
