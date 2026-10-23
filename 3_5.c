#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// 1. Определение статус-кодов
typedef enum {
    SUCCESS = 0,
    ERR_INVALID_ARGS,
    ERR_FILE_OPEN,
    ERR_MEMORY_ALLOC,
    ERR_INVALID_DATA
} StatusCode;

// 2. Структура студента[cite: 20]
typedef struct {
    unsigned int id;
    char* name;
    char* surname;
    char* group;
    unsigned char* grades; // Массив из 5 оценок
} Student;

// --- Валидация данных ---

bool is_latin_string(const char* str) {
    if (!str || *str == '\0') return false;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalpha(str[i])) return false;
    }
    return true;
}

// --- Управление памятью ---

void free_student(Student* s) {
    free(s->name);
    free(s->surname);
    free(s->group);
    free(s->grades);
}

void free_all_students(Student* arr, int count) {
    for (int i = 0; i < count; i++) {
        free_student(&arr[i]);
    }
    free(arr);
}

// --- Чтение базы из файла ---

StatusCode load_students(const char* filepath, Student** out_arr, int* out_count) {
    FILE* file = fopen(filepath, "r");
    if (!file) return ERR_FILE_OPEN;

    int capacity = 10;
    int count = 0;
    Student* arr = (Student*)malloc(capacity * sizeof(Student));
    if (!arr) { fclose(file); return ERR_MEMORY_ALLOC; }

    char t_name[256], t_sur[256], t_grp[256];
    unsigned int t_id;
    unsigned int g[5];

    while (fscanf(file, "%u %255s %255s %255s %u %u %u %u %u", 
           &t_id, t_name, t_sur, t_grp, &g[0], &g[1], &g[2], &g[3], &g[4]) == 9) {
        
        if (!is_latin_string(t_name) || !is_latin_string(t_sur)) {
            continue; // Пропускаем некорректные записи
        }

        if (count == capacity) {
            capacity *= 2;
            Student* temp = (Student*)realloc(arr, capacity * sizeof(Student));
            if (!temp) {
                free_all_students(arr, count);
                fclose(file);
                return ERR_MEMORY_ALLOC;
            }
            arr = temp;
        }

        arr[count].id = t_id;
        arr[count].name = strdup(t_name);
        arr[count].surname = strdup(t_sur);
        arr[count].group = strdup(t_grp);
        arr[count].grades = (unsigned char*)malloc(5 * sizeof(unsigned char));
        for (int i = 0; i < 5; i++) arr[count].grades[i] = (unsigned char)g[i];

        count++;
    }

    fclose(file);
    *out_arr = arr;
    *out_count = count;
    return SUCCESS;
}

// --- Компараторы для qsort[cite: 20] ---

int cmp_id(const void* a, const void* b) {
    return ((Student*)a)->id - ((Student*)b)->id;
}
int cmp_surname(const void* a, const void* b) {
    return strcmp(((Student*)a)->surname, ((Student*)b)->surname);
}
int cmp_name(const void* a, const void* b) {
    return strcmp(((Student*)a)->name, ((Student*)b)->name);
}
int cmp_group(const void* a, const void* b) {
    return strcmp(((Student*)a)->group, ((Student*)b)->group);
}

// --- Вычисления и запись в трассировочный файл[cite: 20] ---

double get_avg(const Student* s) {
    return (s->grades[0] + s->grades[1] + s->grades[2] + s->grades[3] + s->grades[4]) / 5.0;
}

StatusCode write_student_trace(const char* trace_path, const Student* s) {
    FILE* f = fopen(trace_path, "a");
    if (!f) return ERR_FILE_OPEN;
    fprintf(f, "ID: %u | ФИО: %s %s | Группа: %s | Ср.балл: %.2f\n", 
            s->id, s->surname, s->name, s->group, get_avg(s));
    fclose(f);
    return SUCCESS;
}

StatusCode write_top_students(const char* trace_path, Student* arr, int count) {
    if (count == 0) return SUCCESS;
    double total_sum = 0;
    for (int i = 0; i < count; i++) total_sum += get_avg(&arr[i]);
    double overall_avg = total_sum / count;

    FILE* f = fopen(trace_path, "a");
    if (!f) return ERR_FILE_OPEN;
    
    fprintf(f, "--- Студенты со средним баллом выше общего (%.2f) ---\n", overall_avg);
    for (int i = 0; i < count; i++) {
        if (get_avg(&arr[i]) > overall_avg) {
            fprintf(f, "%s %s\n", arr[i].surname, arr[i].name);
        }
    }
    fclose(f);
    return SUCCESS;
}

// --- Интерактивное меню (Начальник) ---

void print_student(const Student* s) {
    printf("ID: %u | %s %s | Гр: %s | Оценки: %d %d %d %d %d\n", 
           s->id, s->surname, s->name, s->group, 
           s->grades[0], s->grades[1], s->grades[2], s->grades[3], s->grades[4]);
}

int main(int argc, char* argv[]) {
    // Входные аргументы: 1 - база студентов, 2 - файл трассировки[cite: 20]
    if (argc != 3) {
        printf("Использование: %s <база.txt> <трассировка.txt>\n", argv[0]);
        return 1;
    }

    Student* students = NULL;
    int count = 0;

    StatusCode status = load_students(argv[1], &students, &count);
    if (status != SUCCESS) {
        printf("Ошибка загрузки файла базы.\n");
        return 1;
    }

    int choice;
    char buffer[256];
    const char* trace_file = argv[2];

    while (1) {
        printf("\n1. Поиск по ID (и запись в trace)\n2. Показать отличников (в trace)\n");
        printf("3. Сортировать (qsort)\n4. Показать всех\n5. Выход\n> ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            unsigned int s_id;
            printf("Введите ID: ");
            scanf("%u", &s_id);
            bool found = false;
            for (int i = 0; i < count; i++) {
                if (students[i].id == s_id) {
                    print_student(&students[i]);
                    write_student_trace(trace_file, &students[i]);
                    printf("Данные записаны в %s\n", trace_file);
                    found = true;
                }
            }
            if (!found) printf("Студент не найден.\n");
        }
        else if (choice == 2) {
            write_top_students(trace_file, students, count);
            printf("Список лучших записан в %s\n", trace_file);
        }
        else if (choice == 3) {
            printf("Сортировать по: 1-ID, 2-Фамилия, 3-Имя, 4-Группа\n> ");
            int s_ch;
            scanf("%d", &s_ch);
            if (s_ch == 1) qsort(students, count, sizeof(Student), cmp_id);
            if (s_ch == 2) qsort(students, count, sizeof(Student), cmp_surname);
            if (s_ch == 3) qsort(students, count, sizeof(Student), cmp_name);
            if (s_ch == 4) qsort(students, count, sizeof(Student), cmp_group);
            printf("База отсортирована.\n");
        }
        else if (choice == 4) {
            for (int i = 0; i < count; i++) print_student(&students[i]);
        }
        else if (choice == 5) {
            break;
        }
    }

    free_all_students(students, count);
    return 0;
}