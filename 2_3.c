#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

// Статус-коды для обработки ошибок
typedef enum {
    SUCCESS = 0,
    ERR_INVALID_ARGS,
    ERR_MEMORY_ALLOCATION,
    ERR_FILE_OPEN,
    ERR_NULL_POINTER
} StatusCode;

// Структура для хранения одного вхождения
typedef struct {
    unsigned int line;
    unsigned int col;
} Occurrence;

// Структура для результатов поиска в одном файле
typedef struct {
    const char* filepath;
    Occurrence* occurrences;
    unsigned int count;
    unsigned int capacity;
    StatusCode status;
} FileResult;

// Пользовательская функция определения длины строки
size_t custom_strlen(const char* str) {
    size_t len = 0;
    while (str && str[len] != '\0') {
        len++;
    }
    return len;
}

// Функция добавления вхождения в динамический массив
StatusCode add_occurrence(FileResult* file_res, unsigned int line, unsigned int col) {
    if (file_res->count >= file_res->capacity) {
        unsigned int new_capacity = file_res->capacity == 0 ? 4 : file_res->capacity * 2;
        Occurrence* new_arr = (Occurrence*)realloc(file_res->occurrences, new_capacity * sizeof(Occurrence));
        if (!new_arr) return ERR_MEMORY_ALLOCATION;
        file_res->occurrences = new_arr;
        file_res->capacity = new_capacity;
    }
    file_res->occurrences[file_res->count].line = line;
    file_res->occurrences[file_res->count].col = col;
    file_res->count++;
    return SUCCESS;
}

// Функция поиска подстроки в одном файле
StatusCode search_in_file(const char* filepath, const char* substring, size_t sub_len, FileResult* result) {
    result->filepath = filepath;
    result->occurrences = NULL;
    result->count = 0;
    result->capacity = 0;
    result->status = SUCCESS;

    FILE* file = fopen(filepath, "r");
    if (!file) {
        result->status = ERR_FILE_OPEN;
        return SUCCESS; // Возвращаем SUCCESS на уровне бизнес-логики, ошибка зафиксирована в структуре файла
    }

    unsigned int current_line = 1;
    unsigned int current_col = 1;
    
    // Переменные для поиска
    int c;
    long start_pos;
    unsigned int start_line, start_col;

    while ((c = fgetc(file)) != EOF) {
        if (c == substring[0]) {
            // Сохраняем текущие координаты
            start_pos = ftell(file);
            start_line = current_line;
            start_col = current_col;

            size_t match_idx = 1;
            bool matched = true;

            // Проверяем совпадение остатка подстроки
            while (match_idx < sub_len) {
                int next_c = fgetc(file);
                if (next_c == EOF || next_c != substring[match_idx]) {
                    matched = false;
                    break;
                }
                match_idx++;
            }

            if (matched) {
                StatusCode add_st = add_occurrence(result, start_line, start_col);
                if (add_st != SUCCESS) {
                    fclose(file);
                    return add_st;
                }
            }

            // Возвращаемся обратно, чтобы не пропустить перекрывающиеся вхождения или продолжить чтение
            fseek(file, start_pos, SEEK_SET);
        }

        if (c == '\n') {
            current_line++;
            current_col = 1;
        } else {
            current_col++;
        }
    }

    fclose(file);
    return SUCCESS;
}

// Функция с переменным числом аргументов (поиск по нескольким файлам)
// Разделяет логику обработки данных от ввода/вывода
StatusCode find_substring_in_files(FileResult** out_results, const char* substring, unsigned int file_count, ...) {
    if (!substring || !out_results || file_count == 0) return ERR_INVALID_ARGS;

    size_t sub_len = custom_strlen(substring);
    if (sub_len == 0) return ERR_INVALID_ARGS;

    *out_results = (FileResult*)malloc(file_count * sizeof(FileResult));
    if (!*out_results) return ERR_MEMORY_ALLOCATION;

    va_list args;
    va_start(args, file_count);

    for (unsigned int i = 0; i < file_count; ++i) {
        const char* filepath = va_arg(args, const char*);
        if (!filepath) {
            (*out_results)[i].filepath = NULL;
            (*out_results)[i].status = ERR_NULL_POINTER;
            continue;
        }
        
        StatusCode status = search_in_file(filepath, substring, sub_len, &(*out_results)[i]);
        if (status != SUCCESS) {
            // В случае критической ошибки памяти очищаем всё, что успели выделить
            for (unsigned int j = 0; j <= i; ++j) {
                if ((*out_results)[j].occurrences) free((*out_results)[j].occurrences);
            }
            free(*out_results);
            *out_results = NULL;
            va_end(args);
            return status;
        }
    }

    va_end(args);
    return SUCCESS;
}

// Функция вывода (вынесена отдельно согласно требованиям)
void print_results(FileResult* results, unsigned int file_count) {
    if (!results) return;

    for (unsigned int i = 0; i < file_count; ++i) {
        printf("Файл: %s\n", results[i].filepath ? results[i].filepath : "NULL");
        
        if (results[i].status == ERR_FILE_OPEN) {
            printf("  -> Ошибка: Не удалось открыть файл.\n");
            continue;
        }
        
        if (results[i].status == ERR_NULL_POINTER) {
            printf("  -> Ошибка: Передан пустой указатель на путь.\n");
            continue;
        }

        if (results[i].count == 0) {
            printf("  -> Вхождений не найдено.\n");
        } else {
            for (unsigned int j = 0; j < results[i].count; ++j) {
                printf("  -> Вхождение: Строка %u, Символ %u\n", 
                       results[i].occurrences[j].line, 
                       results[i].occurrences[j].col);
            }
        }
    }
}

// Функция очистки памяти результатов
void free_results(FileResult* results, unsigned int file_count) {
    if (!results) return;
    for (unsigned int i = 0; i < file_count; ++i) {
        if (results[i].occurrences) {
            free(results[i].occurrences);
            results[i].occurrences = NULL;
        }
    }
    free(results);
}

void create_test_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file) {
        fputs(content, file);
        fclose(file);
    }
}

int main(void) {
    // 1. Писатель создает две тетради с хитрым текстом
    // В первом файле слово "аба" наслаивается друг на друга: "абаба"
    create_test_file("test1.txt", "ababa\nsimple text\naba"); 
    create_test_file("test2.txt", "nothing here");

    const char* substring = "aba";
    unsigned int file_count = 2;
    FileResult* search_results = NULL;
    
    printf("Начинаем тест. Ищем слово: '%s'\n", substring);

    // 2. Начальник отдает команду Менеджеру
    StatusCode status = find_substring_in_files(&search_results, substring, file_count, "test1.txt", "test2.txt");
    
    // 3. Выводим результаты
    if (status == SUCCESS) {
        print_results(search_results, file_count);
        free_results(search_results, file_count);
    } else {
        printf("Произошла ошибка!\n");
    }

    // 4. Убираем за собой (удаляем файлы)
    remove("test1.txt");
    remove("test2.txt");

    return status == SUCCESS ? 0 : 1;
}