import sqlite3
import os
import google.generativeai as genai
import sys

# --- CONFIGURATION ---
DB_PATH = "database/ems.db"  # Path to the SQLite database

# Configure Gemini API
api_key = os.getenv("GEMINI_API_KEY")
if not api_key:
    print("Error: GEMINI_API_KEY environment variable not set.")
    print("Run: export GEMINI_API_KEY='your_key_here'")
    sys.exit(1)

genai.configure(api_key=api_key)

# Use the reliable 'gemini-2.5-flash' model alias
model = genai.GenerativeModel('gemini-2.5-flash')

def fetch_database_context():
    """
    Connects to SQLite, reads all tables, and formats them into a single 
    text string to send to Gemini.
    """
    if not os.path.exists(DB_PATH):
        return f"Error: Database file not found at {DB_PATH}"

    try:
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()

        # 1. Fetch Employees
        cursor.execute("SELECT * FROM employee")
        employees = cursor.fetchall()
        emp_header = "id, name, department, position, hire_date, base_salary"
        emp_str = "\n".join([str(row) for row in employees])

        # 2. Fetch Salaries (Limited to 100 recent records for context efficiency)
        cursor.execute("SELECT * FROM salary LIMIT 100")
        salaries = cursor.fetchall()
        sal_header = "id, employee_id, month, base, bonus, deduction"
        sal_str = "\n".join([str(row) for row in salaries])

        # 3. Fetch Attendance (Only fetching last 50 records)
        cursor.execute("SELECT * FROM attendance ORDER BY id DESC LIMIT 50")
        attendance = cursor.fetchall()
        att_header = "id, employee_id, date, status, note"
        att_str = "\n".join([str(row) for row in attendance])

        conn.close()

        # Construct the context string
        context_data = f"""
        Here is the raw data from the company's SQLite database.
        
        TABLE: EMPLOYEE (Columns: {emp_header})
        DATA:
        {emp_str}

        TABLE: SALARY (Columns: {sal_header})
        DATA:
        {sal_str}

        TABLE: ATTENDANCE (Columns: {att_header})
        DATA:
        {att_str}
        """
        return context_data

    except Exception as e:
        return f"Error reading database: {e}"

def chat_loop():
    print("--- 🤖 AI-Driven Employee Management Assistant (Gemini) ---")
    print(f"Loading data from {DB_PATH}...")
    
    db_context = fetch_database_context()
    
    if "Error" in db_context:
        print(db_context)
        return

    print("Data loaded successfully! Ask questions about employees, salaries, or attendance.")
    print("Type 'quit' or 'exit' to stop.\n")

    # Start a chat session with history
    chat = model.start_chat(history=[])

    # System instruction prompt (sets the AI's persona and rules)
    system_prompt = f"""
    You are an intelligent HR Data Analyst for a company. Your task is to analyze the raw database data provided below.
    
    {db_context}

    INSTRUCTIONS:
    1. Answer the user's questions based ONLY on the data provided above.
    2. If the user asks for analysis (e.g., average, comparison), calculate it based on the data.
    3. **Always respond in English**, unless the user explicitly requests another language.
    4. Be professional and concise.
    """
    
    # Prime the model with the data
    try:
        chat.send_message(system_prompt)
    except Exception as e:
        print(f"Failed to connect to Gemini. Check your network or API Key: {e}")
        return

    while True:
        user_input = input("\nEnter your question > ")
        if user_input.lower() in ["quit", "exit"]:
            break
        
        if not user_input.strip():
            continue

        try:
            print("AI is thinking...")
            response = chat.send_message(user_input)
            print(f"\n🤖 AI Response > {response.text}")
        except Exception as e:
            print(f"An error occurred: {e}")

if __name__ == "__main__":
    chat_loop()