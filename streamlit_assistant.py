import os
import sqlite3
import pandas as pd
import streamlit as st

# Gemini SDK
import google.generativeai as genai


# ==========================
# CONFIGURATION
# ==========================
DB_PATH = "database/ems.db"
MODEL_NAME = "gemini-2.0-flash"   # or another model you prefer


# ==========================
# HELPERS: DATABASE
# ==========================
@st.cache_data(show_spinner=False)
def load_data():
    """Load employee, attendance, and salary tables into DataFrames."""
    conn = sqlite3.connect(DB_PATH)

    employees = pd.read_sql_query(
        "SELECT id, name, department, position, hire_date, base_salary "
        "FROM employee;", conn
    )

    # Attendance table
    attendance = pd.read_sql_query(
        "SELECT id, employee_id, date, status, note FROM attendance;",
        conn,
        parse_dates=["date"],
    )

    # Salary table (optional – if empty, code will handle it)
    try:
        salary = pd.read_sql_query(
            "SELECT id, employee_id, month, base_salary, bonus, deduction, total "
            "FROM salary;",
            conn,
        )
    except Exception:
        salary = pd.DataFrame(
            columns=["id", "employee_id", "month", "base_salary", "bonus", "deduction", "total"]
        )

    conn.close()
    return employees, attendance, salary


def build_data_summary(employees: pd.DataFrame,
                       attendance: pd.DataFrame,
                       salary: pd.DataFrame) -> str:
    """
    Build a compact text summary of the EMS data to send to Gemini.
    This helps the model answer questions grounded in your real tables.
    """
    lines = []
    total_emp = len(employees)
    num_dept = employees["department"].nunique()

    lines.append(f"There are {total_emp} employees across {num_dept} departments.")
    dept_summary = (
        employees.groupby("department")
        .agg(
            headcount=("id", "count"),
            avg_salary=("base_salary", "mean"),
        )
        .reset_index()
    )

    lines.append("Department summary (name, headcount, average base salary):")
    for _, row in dept_summary.iterrows():
        lines.append(
            f"- {row['department']}: {int(row['headcount'])} employees, "
            f"average salary ≈ {row['avg_salary']:.0f}"
        )

    # Attendance overview
    if not attendance.empty:
        status_counts = attendance["status"].value_counts()
        total_att = int(status_counts.sum())
        lines.append(
            f"There are {total_att} attendance records "
            f"with statuses such as {', '.join(status_counts.index.tolist())}."
        )
        for status, cnt in status_counts.items():
            lines.append(f"- {status}: {int(cnt)} records")

        # Join with employees to get department-level attendance
        att_merged = attendance.merge(
            employees[["id", "department", "name"]],
            left_on="employee_id",
            right_on="id",
            how="left",
        )
        dept_att = (
            att_merged.groupby(["department", "status"])["id_x"]
            .count()
            .unstack(fill_value=0)
        )

        lines.append("Department-level attendance (approximate counts):")
        for dept in dept_att.index:
            row = dept_att.loc[dept]
            parts = [f"{status}={int(row[status])}" for status in row.index]
            lines.append(f"- {dept}: " + ", ".join(parts))

    # Salary overview (if available)
    if not salary.empty:
        max_total = salary["total"].max()
        min_total = salary["total"].min()
        lines.append(
            f"Salaries table includes monthly records. "
            f"Max total salary ≈ {max_total:.0f}, min total salary ≈ {min_total:.0f}."
        )

    return "\n".join(lines)


def configure_gemini():
    """Configure Gemini model using environment variable GEMINI_API_KEY."""
    api_key = os.getenv("GEMINI_API_KEY")
    if not api_key:
        st.error(
            "GEMINI_API_KEY is not set in your environment. "
            "Please export GEMINI_API_KEY before running this app."
        )
        st.stop()

    genai.configure(api_key=api_key)
    model = genai.GenerativeModel(MODEL_NAME)
    return model


# ==========================
# STREAMLIT LAYOUT
# ==========================
def main():
    st.set_page_config(
        page_title="AI-Driven Employee Management Assistant",
        page_icon="💼",
        layout="wide",
    )

    st.title("💼 AI-Driven Employee Management Assistant (Gemini)")
    st.markdown(
        """
This web interface connects to the local **ems.db** database and uses **Gemini**
to perform natural language analysis on employees, salaries, and attendance data.

The page is divided into two parts:

1. **Data Overview** – visual analytics and key metrics  
2. **Gemini AI Analysis** – ask free-form questions about the company data
        """
    )

    # Load data
    employees, attendance, salary = load_data()

    # ------------- KPIs -------------
    st.subheader("📊 Key Metrics")

    col1, col2, col3, col4 = st.columns(4)

    total_employees = len(employees)
    num_departments = employees["department"].nunique()
    avg_base_salary = employees["base_salary"].mean() if not employees.empty else 0
    it_count = len(employees[employees["department"] == "IT"])

    col1.metric("Total Employees", total_employees)
    col2.metric("Number of Departments", num_departments)
    col3.metric("Average Base Salary", f"{avg_base_salary:,.0f}")
    col4.metric("Employees in IT", it_count)

    st.markdown("---")

    # ------------- Department overview -------------
    st.subheader("🏢 Department Overview")

    dept_summary = (
        employees.groupby("department")
        .agg(
            headcount=("id", "count"),
            avg_salary=("base_salary", "mean"),
        )
        .reset_index()
        .sort_values("headcount", ascending=False)
    )

    left, right = st.columns(2)

    with left:
        st.markdown("#### Headcount by Department")
        st.dataframe(dept_summary, use_container_width=True)
        st.bar_chart(
            dept_summary.set_index("department")["headcount"],
            use_container_width=True,
        )

    with right:
        st.markdown("#### Average Base Salary by Department")
        st.bar_chart(
            dept_summary.set_index("department")["avg_salary"],
            use_container_width=True,
        )

    st.markdown("---")

    # ------------- Attendance analytics -------------
    st.subheader("🕒 Attendance Analytics")

    if attendance.empty:
        st.info("No attendance records found in the database.")
    else:
        # Overall status distribution
        status_counts = attendance["status"].value_counts()
        col_a, col_b = st.columns(2)

        with col_a:
            st.markdown("#### Attendance Status Distribution")
            st.bar_chart(status_counts, use_container_width=True)

        # Department-level attendance rate
        att_merged = attendance.merge(
            employees[["id", "department"]],
            left_on="employee_id",
            right_on="id",
            how="left",
        )

        dept_att = (
            att_merged.groupby(["department", "status"])["id_x"]
            .count()
            .unstack(fill_value=0)
        )

        # consider "Present" as the main positive status
        total_by_dept = dept_att.sum(axis=1)
        present = dept_att.get("Present", 0)
        present_rate = (present / total_by_dept.replace(0, pd.NA) * 100).fillna(0)

        with col_b:
            st.markdown("#### Present Rate by Department (%)")
            st.bar_chart(present_rate, use_container_width=True)

        # Optional: show raw attendance table in an expander
        with st.expander("Show raw attendance records (sample)", expanded=False):
            st.write(attendance.head(50))

    st.markdown("---")

    # ------------- Salary distribution -------------
    st.subheader("💰 Salary Distribution")

    if salary.empty:
        st.info("No detailed monthly salary records found. Using base_salary only.")
        base_salaries = employees["base_salary"]
    else:
        base_salaries = employees["base_salary"]

    st.markdown("Histogram of base salaries (from employee table):")
    st.bar_chart(base_salaries.value_counts().sort_index(), use_container_width=True)

    st.markdown("---")

    # ======================================
    # GEMINI NATURAL LANGUAGE ANALYSIS
    # ======================================
    st.subheader("🤖 Gemini Natural Language Analysis")

    model = configure_gemini()
    data_summary = build_data_summary(employees, attendance, salary)

    # Session state for chat history
    if "ai_history" not in st.session_state:
        st.session_state["ai_history"] = []

    with st.expander("What can I ask?", expanded=False):
        st.markdown(
            """
You can ask questions such as:

- **"Which department has the highest average salary?"**  
- **"Who are the top 5 highest-paid employees in IT?"**  
- **"Summarize the attendance issues in the last month."**  
- **"Compare the average salary between Sales and IT."**  
- **"Give me an English summary of the staffing structure."**
            """
        )

    # Optional: show data summary that will be sent to Gemini
    with st.expander("Show compact data summary sent to Gemini"):
        st.text(data_summary)

    # User input
    st.markdown("#### Ask Gemini a question about the company data")
    user_question = st.text_area(
        "Your question",
        value="Please summarize the company staffing structure in a short paragraph.",
        height=80,
    )

    if st.button("🚀 Send to Gemini"):
        if not user_question.strip():
            st.warning("Please enter a question first.")
        else:
            with st.spinner("Gemini is thinking..."):
                prompt = (
                    "You are an AI assistant helping with analysis of an employee "
                    "management system. Use ONLY the provided data summary to answer, "
                    "and do not invent employees that are not described.\n\n"
                    "=== DATA SUMMARY START ===\n"
                    f"{data_summary}\n"
                    "=== DATA SUMMARY END ===\n\n"
                    f"User question: {user_question}"
                )

                try:
                    response = model.generate_content(prompt)
                    answer = response.text
                except Exception as e:
                    answer = f"Error while calling Gemini: {e}"

            # Save to history
            st.session_state["ai_history"].append(
                {"question": user_question, "answer": answer}
            )

    # Show history
    if st.session_state["ai_history"]:
        st.markdown("#### AI Query History")
        for i, item in enumerate(reversed(st.session_state["ai_history"]), start=1):
            st.markdown(f"**Q{i}:** {item['question']}")
            st.markdown(f"**A{i}:** {item['answer']}")
            st.markdown("---")


if __name__ == "__main__":
    main()
