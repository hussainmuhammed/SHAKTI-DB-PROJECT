#include <stdio.h>
#include <postgresql/libpq-fe.h>
#include <stdlib.h>

void viewAttendance() {
    PGconn *conn;
    PGresult *res;
    int rows, i;
    float percentage;

    conn = PQconnectdb("dbname=studentdb user=postgres");

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    res = PQexec(conn,
        "SELECT s.id, s.name, "
        "COUNT(*) AS total_days, "
        "SUM(CASE WHEN a.status='Present' THEN 1 ELSE 0 END) AS present_days, "
        "ROUND(100.0 * "
        "SUM(CASE WHEN a.status='Present' THEN 1 ELSE 0 END) "
        "/ COUNT(*), 2) AS attendance_percentage "
        "FROM students s "
        "JOIN attendance a ON s.id = a.student_id "
        "GROUP BY s.id, s.name "
        "ORDER BY s.id");

    rows = PQntuples(res);

    printf("\n===== ATTENDANCE REPORT =====\n\n");

    for(i = 0; i < rows; i++) {

        percentage = atof(PQgetvalue(res, i, 4));

        printf("ID         : %s\n", PQgetvalue(res, i, 0));
        printf("Name       : %s\n", PQgetvalue(res, i, 1));
        printf("Total Days : %s\n", PQgetvalue(res, i, 2));
        printf("Present    : %s\n", PQgetvalue(res, i, 3));
        printf("Attendance : %.2f%%\n", percentage);

        if (percentage < 75)
            printf("Remark     : Not Eligible for Exam\n");
        else
            printf("Remark     : Eligible for Exam\n");

        printf("-------------------------\n");
    }

    PQclear(res);
    PQfinish(conn);
}
void studentReport() {
    PGconn *conn;
    PGresult *res;
    int id;

    conn = PQconnectdb("dbname=studentdb user=postgres");

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    printf("Enter Student ID: ");
    scanf("%d", &id);

    char query[1000];

    sprintf(query,
        "SELECT s.id, s.name, s.department, s.cgpa, "
        "a.status, m.subject, m.marks "
        "FROM students s "
        "LEFT JOIN attendance a ON s.id = a.student_id "
        "LEFT JOIN marks m ON s.id = m.student_id "
        "WHERE s.id = %d",
        id);

    res = PQexec(conn, query);

    if (PQntuples(res) == 0) {
        printf("Student Not Found!\n");
    } else {

        printf("\n===== STUDENT REPORT =====\n");

        printf("ID         : %s\n",
               PQgetvalue(res,0,0));

        printf("Name       : %s\n",
               PQgetvalue(res,0,1));

        printf("Department : %s\n",
               PQgetvalue(res,0,2));

        printf("CGPA       : %s\n",
               PQgetvalue(res,0,3));

        printf("Attendance : %s\n",
               PQgetvalue(res,0,4));

        printf("Subject    : %s\n",
               PQgetvalue(res,0,5));

        printf("Marks      : %s\n",
               PQgetvalue(res,0,6));
    }

    PQclear(res);
    PQfinish(conn);
}
void showTopper() {
    PGconn *conn;
    PGresult *res;

    conn = PQconnectdb("dbname=studentdb user=postgres");

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    res = PQexec(conn,
        "SELECT * FROM students "
        "ORDER BY cgpa DESC "
        "LIMIT 1");

    if (PQntuples(res) > 0) {
        printf("\n===== TOPPER =====\n");

        printf("ID         : %s\n",
               PQgetvalue(res,0,0));

        printf("Name       : %s\n",
               PQgetvalue(res,0,1));

        printf("Department : %s\n",
               PQgetvalue(res,0,2));

        printf("CGPA       : %s\n",
               PQgetvalue(res,0,3));
    }

    PQclear(res);
    PQfinish(conn);
}
void deleteStudent() {
    PGconn *conn;
    PGresult *res;

    int id;
    char query[200];

    conn = PQconnectdb("dbname=studentdb user=postgres");

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    printf("Enter Student ID to delete: ");
    scanf("%d", &id);

    /* Delete attendance records */
    sprintf(query,
            "DELETE FROM attendance WHERE student_id=%d",
            id);
    PQexec(conn, query);

    /* Delete marks records */
    sprintf(query,
            "DELETE FROM marks WHERE student_id=%d",
            id);
    PQexec(conn, query);

    /* Delete student record */
    sprintf(query,
            "DELETE FROM students WHERE id=%d",
            id);

    res = PQexec(conn, query);

    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        printf("Student Deleted Successfully!\n");
    else
        printf("Error: %s\n", PQerrorMessage(conn));

    PQclear(res);
    PQfinish(conn);
}
void updateStudent() {
PGconn *conn;
PGresult *res;


int id, marks;
char name[50];
char dept[50];
char subject[50];
float cgpa;
char query[500];

conn = PQconnectdb("dbname=studentdb user=postgres");

if (PQstatus(conn) != CONNECTION_OK) {
    printf("Connection failed: %s\n", PQerrorMessage(conn));
    PQfinish(conn);
    return;
}

printf("Enter Student ID: ");
scanf("%d", &id);

printf("Enter New Name: ");
scanf(" %49[^\n]", name);

printf("Enter New Department: ");
scanf("%49s", dept);

printf("Enter New CGPA: ");
scanf("%f", &cgpa);

printf("Enter Subject: ");
scanf(" %49[^\n]", subject);

printf("Enter Marks: ");
scanf("%d", &marks);

sprintf(query,
        "UPDATE students SET name='%s', department='%s', cgpa=%f WHERE id=%d",
        name, dept, cgpa, id);
PQexec(conn, query);

sprintf(query,
        "DELETE FROM marks WHERE student_id=%d",
        id);
PQexec(conn, query);

sprintf(query,
        "INSERT INTO marks VALUES (%d, '%s', %d)",
        id, subject, marks);

res = PQexec(conn, query);

if (PQresultStatus(res) == PGRES_COMMAND_OK)
    printf("Student Updated Successfully!\n");
else
    printf("Error: %s\n", PQerrorMessage(conn));

PQclear(res);
PQfinish(conn);


}


void viewMarks() {
    PGconn *conn;
    PGresult *res;
    int rows, i;

    conn = PQconnectdb("dbname=studentdb user=postgres");

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    res = PQexec(conn,
        "SELECT s.id, s.name, m.subject, m.marks "
        "FROM students s "
        "JOIN marks m ON s.id = m.student_id "
        "ORDER BY s.id");

    rows = PQntuples(res);

    printf("\n===== MARKS =====\n");

    for(i = 0; i < rows; i++) {

        printf("ID      : %s\n",
               PQgetvalue(res,i,0));

        printf("Name    : %s\n",
               PQgetvalue(res,i,1));

        printf("Subject : %s\n",
               PQgetvalue(res,i,2));

        printf("Marks   : %s\n",
               PQgetvalue(res,i,3));

        printf("-------------------------\n");
    }

    PQclear(res);
    PQfinish(conn);
}
void addStudent() {
    PGconn *conn;
    PGresult *res;

    int id;
    char name[50];
    char dept[50];
    float cgpa;
    char query[500];

    conn = PQconnectdb("dbname=studentdb user=postgres");

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    printf("Enter ID: ");
    scanf("%d", &id);

    printf("Enter Name: ");
    scanf("%49s", name);

    printf("Enter Department: ");
    scanf("%49s", dept);

    printf("Enter CGPA: ");
    scanf("%f", &cgpa);

    sprintf(query,
            "INSERT INTO students VALUES (%d,'%s','%s',%f)",
            id, name, dept, cgpa);

    res = PQexec(conn, query);

    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        printf("Student Added Successfully!\n");
    else
        printf("Error: %s\n", PQerrorMessage(conn));

    PQclear(res);
    PQfinish(conn);
}
void searchStudent() {
    PGconn *conn;
    PGresult *res;
    int rows, i;
    int choice, id;
    char name[50];
    char dept[50];
    char query[300];

    conn = PQconnectdb("dbname=studentdb user=postgres");

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    printf("\n1. Search by ID\n");
    printf("2. Search by Name\n");
    printf("3. Search by Department\n");
    printf("Enter Choice: ");
    scanf("%d", &choice);

    if (choice == 1) {

        printf("Enter ID: ");
        scanf("%d", &id);

        sprintf(query,
                "SELECT * FROM students WHERE id=%d",
                id);
    }

    else if (choice == 2) {

        printf("Enter Name: ");
        scanf(" %49[^\n]", name);

        sprintf(query,
                "SELECT * FROM students "
                "WHERE name ILIKE '%%%s%%' "
                "ORDER BY id",
                name);
    }

    else if (choice == 3) {

        printf("Enter Department: ");
        scanf("%49s", dept);

        sprintf(query,
                "SELECT * FROM students "
                "WHERE department='%s' "
                "ORDER BY id",
                dept);
    }

    else {
        printf("Invalid Choice!\n");
        PQfinish(conn);
        return;
    }

    res = PQexec(conn, query);

    rows = PQntuples(res);

    if (rows == 0) {
        printf("No Student Found!\n");
    }
    else {

        printf("\n===== SEARCH RESULT =====\n");

        for (i = 0; i < rows; i++) {

            printf("ID: %s\n",
                   PQgetvalue(res, i, 0));

            printf("Name: %s\n",
                   PQgetvalue(res, i, 1));

            printf("Department: %s\n",
                   PQgetvalue(res, i, 2));

            printf("CGPA: %s\n",
                   PQgetvalue(res, i, 3));

            printf("-------------------------\n");
        }
    }

    PQclear(res);
    PQfinish(conn);
}
void viewStudents() {
    PGconn *conn;
    PGresult *res;
    int rows, i;

    conn = PQconnectdb("dbname=studentdb user=postgres");

    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    res = PQexec(conn,
                 "SELECT * FROM students ORDER BY id");

    rows = PQntuples(res);

    printf("\n===== STUDENTS =====\n");

    for(i = 0; i < rows; i++) {

        printf("ID: %s\n",
               PQgetvalue(res, i, 0));

        printf("Name: %s\n",
               PQgetvalue(res, i, 1));

        printf("Department: %s\n",
               PQgetvalue(res, i, 2));

        printf("CGPA: %s\n",
               PQgetvalue(res, i, 3));

        printf("-------------------------\n");
    }

    PQclear(res);
    PQfinish(conn);
}
int main() {
    int choice;

    while(1) {

        printf("\n===== STUDENT MANAGEMENT SYSTEM =====\n");
	printf("1. View Students\n");
	printf("2. Add Student\n");
	printf("3. Search Student\n");
	printf("4. Update Student\n");
	printf("5. Delete Student\n");
	printf("6. View Attendence\n");
	printf("7. View Marks\n");
	printf("8. Show Topper\n");
	printf("9. Student Report\n");
        printf("10.Exit\n");
	printf("Enter Choice: ");
        scanf("%d", &choice);

        switch(choice) {

            case 1:
                viewStudents();
                break;
	    case 2:
        	addStudent();
        	break;
	    case 3:
    		searchStudent();
    		break;

	    case 4:
    		updateStudent();
    		break;
   	    case 5:
    		deleteStudent();
                break;

            case 6:
                viewAttendance();
                break;

	    case 7:
                viewMarks();
                break;

            case 8:
    		showTopper();
    		break;
	    case 9:
    		studentReport();
    		break;

	    case 10:
    		printf("Exiting...\n");
    		return 0;

            default:
                printf("Feature coming next...\n");
        }
    }

    return 0;
}
