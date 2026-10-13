#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


typedef enum {
    SUCCESS = 0,
    ERR_INVALID_ARGS,
    ERR_UNKNOWN_FLAG,
    ERR_FILE_OPEN_INPUT,
    ERR_FILE_OPEN_OUTPUT,
    ERR_MEMORY_ALLOC,
    ERR_INVALID_DATA
} StatusCode;


typedef struct {
    unsigned int id;
    char first_name[128];
    char last_name[128];
    double salary;
} Employee;




bool is_valid_name(const char* name) {
    if (!name || name[0] == '\0') return false;
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalpha((unsigned char)name[i])) return false;
    }
    return true;
}

int compare_doubles(double a, double b, double eps) {
    if (a - b > eps) return 1;
    if (b - a > eps) return -1;
    return 0;
}

int cmp_asc(const void* a, const void* b) {
    const Employee* e1 = (const Employee*)a;
    const Employee* e2 = (const Employee*)b;
    const double eps = 1e-6; 


    int s_cmp = compare_doubles(e1->salary, e2->salary, eps);
    if (s_cmp != 0) return s_cmp;

    int l_cmp = strcmp(e1->last_name, e2->last_name);
    if (l_cmp != 0) return l_cmp;

    int f_cmp = strcmp(e1->first_name, e2->first_name);
    if (f_cmp != 0) return f_cmp;

    if (e1->id > e2->id) return 1;
    if (e1->id < e2->id) return -1;
    return 0;
}

int cmp_desc(const void* a, const void* b) {
    const Employee* e1 = (const Employee*)a;
    const Employee* e2 = (const Employee*)b;
    const double eps = 1e-6;

    int s_cmp = compare_doubles(e2->salary, e1->salary, eps);
    if (s_cmp != 0) return s_cmp;

    int l_cmp = strcmp(e2->last_name, e1->last_name);
    if (l_cmp != 0) return l_cmp;

    int f_cmp = strcmp(e2->first_name, e1->first_name);
    if (f_cmp != 0) return f_cmp;

    if (e1->id < e2->id) return 1;
    if (e1->id > e2->id) return -1;
    return 0;
}

void sort_employees(Employee* arr, size_t count, bool ascending) {
    if (count == 0) return;
    
    if (ascending) {
        qsort(arr, count, sizeof(Employee), cmp_asc);
    } else {
        qsort(arr, count, sizeof(Employee), cmp_desc);
    }
}


StatusCode read_employees(const char* filepath, Employee** employees_out, size_t* count_out) {
    FILE* file = fopen(filepath, "r");
    if (!file) return ERR_FILE_OPEN_INPUT;

    size_t capacity = 10;
    size_t count = 0;
    Employee* arr = (Employee*)malloc(capacity * sizeof(Employee));
    if (!arr) {
        fclose(file);
        return ERR_MEMORY_ALLOC;
    }

    char fname[128];
    char lname[128];
    long long temp_id;
    double temp_salary;

    while (fscanf(file, "%lld %127s %127s %lf", &temp_id, fname, lname, &temp_salary) == 4) {
        if (temp_id < 0 || temp_salary < 0.0 || !is_valid_name(fname) || !is_valid_name(lname)) {
            free(arr);
            fclose(file);
            return ERR_INVALID_DATA;
        }

        if (count >= capacity) {
            capacity *= 2;
            Employee* temp = (Employee*)realloc(arr, capacity * sizeof(Employee));
            if (!temp) {
                free(arr);
                fclose(file);
                return ERR_MEMORY_ALLOC;
            }
            arr = temp;
        }

        arr[count].id = (unsigned int)temp_id;
        strcpy(arr[count].first_name, fname);
        strcpy(arr[count].last_name, lname);
        arr[count].salary = temp_salary;
        count++;
    }
    if (!feof(file)) {
        int c;
        while ((c = fgetc(file)) != EOF) {
            if (!isspace(c)) {
                free(arr);
                fclose(file);
                return ERR_INVALID_DATA;
            }
        }
    }

    fclose(file);
    *employees_out = arr;
    *count_out = count;
    return SUCCESS;
}

StatusCode write_employees(const char* filepath, const Employee* arr, size_t count) {
    FILE* file = fopen(filepath, "w");
    if (!file) return ERR_FILE_OPEN_OUTPUT;

    for (size_t i = 0; i < count; i++) {
        fprintf(file, "%u %s %s %.2lf\n", arr[i].id, arr[i].first_name, arr[i].last_name, arr[i].salary);
    }

    fclose(file);
    return SUCCESS;
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Ошибка: неверное количество аргументов.\nИспользование: %s <входной_файл> <флаг> <выходной_файл>\n", argv[0]);
        return 1;
    }

    const char* in_filepath = argv[1];
    const char* flag = argv[2];
    const char* out_filepath = argv[3];

    if (strlen(flag) != 2 || (flag[0] != '-' && flag[0] != '/') || (flag[1] != 'a' && flag[1] != 'd')) {
        fprintf(stderr, "Ошибка: неверный формат флага. Ожидается -a, /a, -d или /d.\n");
        return 2;
    }

    bool ascending = (flag[1] == 'a');

    Employee* employees = NULL;
    size_t count = 0;
    StatusCode status;

    status = read_employees(in_filepath, &employees, &count);
    if (status == SUCCESS) {
        sort_employees(employees, count, ascending);
        status = write_employees(out_filepath, employees, count);
        free(employees); 
    }

    switch (status) {
        case SUCCESS:
            printf("Обработка успешно завершена.\n");
            return 0;
        case ERR_FILE_OPEN_INPUT:
            fprintf(stderr, "Ошибка: не удалось открыть входной файл %s.\n", in_filepath);
            return 3;
        case ERR_FILE_OPEN_OUTPUT:
            fprintf(stderr, "Ошибка: не удалось создать выходной файл %s.\n", out_filepath);
            return 4;
        case ERR_MEMORY_ALLOC:
            fprintf(stderr, "Ошибка: не удалось выделить память.\n");
            return 5;
        case ERR_INVALID_DATA:
            fprintf(stderr, "Ошибка: данные в файле невалидны (отрицательные числа или цифры/спецсимволы в именах).\n");
            return 6;
        default:
            fprintf(stderr, "Неизвестная ошибка.\n");
            return -1;
    }
}