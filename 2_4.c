#include <stdio.h>
#include <stdlib.h>


typedef enum {
    SUCCESS = 0,
    ERR_INVALID_ARGS,
    ERR_MEMORY_ALLOC,
    ERR_INVALID_INPUT
} StatusCode;


typedef struct {
    char* filename;
    unsigned int size;
    unsigned int importance;
} File;


typedef struct {
    File* selected_files;
    unsigned int total_size;
    unsigned int total_importance;
    unsigned int count;
} BackupResult;

//копирование строк 
char* custom_strdup(const char* src) {
    if (!src) return NULL;
    unsigned int len = 0;
    while (src[len] != '\0') len++;
    
    char* dest = (char*)malloc(len + 1);
    if (!dest) return NULL;
    
    for (unsigned int i = 0; i <= len; i++) {
        dest[i] = src[i];
    }
    return dest;
}

// Функция-планировщик (решает классическую задачу о рюкзаке)
StatusCode select_backup_files(const File* files, unsigned int n, unsigned int capacity, BackupResult* result) {
    if (!files || !result) return ERR_INVALID_ARGS;

    result->selected_files = NULL;
    result->total_size = 0;
    result->total_importance = 0;
    result->count = 0;

    if (n == 0 || capacity == 0) return SUCCESS;

    // 1. Выделяем память для таблицы расчетов
    unsigned int* dp = (unsigned int*)calloc(capacity + 1, sizeof(unsigned int));
    if (!dp) return ERR_MEMORY_ALLOC;

    // Таблица "закладок", чтобы вспомнить, какие именно файлы мы взяли
    unsigned char** keep = (unsigned char**)malloc(n * sizeof(unsigned char*));
    if (!keep) {
        free(dp);
        return ERR_MEMORY_ALLOC;
    }

    for (unsigned int i = 0; i < n; i++) {
        keep[i] = (unsigned char*)calloc(capacity + 1, sizeof(unsigned char));
        if (!keep[i]) {
            for (unsigned int j = 0; j < i; j++) free(keep[j]);
            free(keep);
            free(dp);
            return ERR_MEMORY_ALLOC;
        }
    }

    // 2. Основной алгоритм: ищем самую выгодную комбинацию
    for (unsigned int i = 0; i < n; i++) {
        for (unsigned int w = capacity; w >= files[i].size; w--) {
            // Если добавить текущий файл выгоднее, чем не добавлять:
            if (dp[w - files[i].size] + files[i].importance > dp[w]) {
                dp[w] = dp[w - files[i].size] + files[i].importance;
                keep[i][w] = 1; // Делаем пометку "взяли"
            }
        }
    }

    // 3. Восстанавливаем список выбранных файлов
    unsigned int current_capacity = capacity;
    unsigned char* is_selected = (unsigned char*)calloc(n, sizeof(unsigned char));
    if (!is_selected) {

        for (unsigned int i = 0; i < n; i++) free(keep[i]);
        free(keep);
        free(dp);
        return ERR_MEMORY_ALLOC;
    }

    unsigned int selected_count = 0;
    // Идем с конца и смотрим на закладки
    for (int i = (int)n - 1; i >= 0; i--) {
        if (keep[i][current_capacity]) {
            is_selected[i] = 1;
            selected_count++;
            current_capacity -= files[i].size;
        }
    }

    // 4. Заполняем итоговую структуру
    if (selected_count > 0) {
        result->selected_files = (File*)malloc(selected_count * sizeof(File));
        if (!result->selected_files) {
            free(is_selected);
            for (unsigned int i = 0; i < n; i++) free(keep[i]);
            free(keep);
            free(dp);
            return ERR_MEMORY_ALLOC;
        }

        unsigned int res_idx = 0;
        for (unsigned int i = 0; i < n; i++) {
            if (is_selected[i]) {
                result->selected_files[res_idx].filename = custom_strdup(files[i].filename);
                result->selected_files[res_idx].size = files[i].size;
                result->selected_files[res_idx].importance = files[i].importance;
                
                result->total_size += files[i].size;
                result->total_importance += files[i].importance;
                result->count++;
                res_idx++;
            }
        }
    }

    // 5. Очищаем все черновики
    free(is_selected);
    for (unsigned int i = 0; i < n; i++) free(keep[i]);
    free(keep);
    free(dp);

    return SUCCESS;
}

// Функция вывода результатов
void print_results(const BackupResult* result) {
    printf("TOTAL_SIZE %u\n", result->total_size);
    printf("TOTAL_IMPORTANCE %u\n", result->total_importance);
    printf("FILES %u\n", result->count);
    
    for (unsigned int i = 0; i < result->count; i++) {
        printf("%s\n", result->selected_files[i].filename);
    }
}

// Зачистка исходного массива и результатов
void free_all(File* files, unsigned int n, BackupResult* result) {
    if (files) {
        for (unsigned int i = 0; i < n; i++) free(files[i].filename);
        free(files);
    }
    if (result && result->selected_files) {
        for (unsigned int i = 0; i < result->count; i++) free(result->selected_files[i].filename);
        free(result->selected_files);
    }
}

int main(void) {
    unsigned int n, capacity;
    
    if (scanf("%u %u", &n, &capacity) != 2) {
        printf("Ошибка: неверный формат ввода.\n");
        return 1;
    }

    File* files = NULL;
    if (n > 0) {
        files = (File*)malloc(n * sizeof(File));
        if (!files) return 1;

        char buffer[256];
        for (unsigned int i = 0; i < n; i++) {
            if (scanf("%255s %u %u", buffer, &files[i].size, &files[i].importance) != 3) {
                printf("Ошибка: неверный формат описания файла.\n");
                for (unsigned int j = 0; j < i; j++) free(files[j].filename);
                free(files);
                return 1;
            }
            files[i].filename = custom_strdup(buffer);
        }
    }

    BackupResult result = {0};
    StatusCode status = select_backup_files(files, n, capacity, &result);

    switch (status) {
        case SUCCESS:
            print_results(&result);
            break;
        case ERR_MEMORY_ALLOC:
            printf("Ошибка: не хватило памяти.\n");
            break;
        default:
            printf("Неизвестная ошибка.\n");
            break;
    }

    free_all(files, n, &result);
    return status == SUCCESS ? 0 : 1;
}