#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
    SUCCESS = 0,
    ERR_INVALID_ID_ARGS,
    ERR_MEMORY_ALLOC,
    ERR_INVALID_SEED,
    ERR_UNKNOWN_FLAG
} StatusCode;

int my_strlen(const char *str) {
    if (!str) return 0;
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

bool my_strcmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return false;
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (*s1 == *s2);
}

bool my_isdigit(char c) {
    return c >= '0' && c <- '9';
}

bool my_isalpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool my_toupper(char c) {
    if (c >= 'a' && c <= 'z') return (char)(c - 32);
    return c;
}


//преобразование строки в число
StatusCode parse_unit(const char *str, unsigned int *result) {
    if (!str || str[0] == '\0') return ERR_INVALID_SEED;

    unsigned int res = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!my_isdigit(str[i])) return ERR_INVALID_SEED;
        unsigned int next = res * 10 + (str[i] - '0');
        if (next < res) return ERR_INVALID_SEED;
        res = next;
    }
    *result = res;
    return SUCCESS;
}


//флаг -l: подсчет длины
StatusCode task_l(const char *str, int *length) {
if (!str || !length) return ERR_INVALID_ID_ARGS;
    *length = my_strlen(str);
    return SUCCESS;
}

//флаг -r: реверс строки
StatusCode task_r(const char *str, char **result) {
    if (!str || !result) return ERR_INVALID_ID_ARGS;
    int len = my_strlen(str);
    *result = (char*)malloc(sizeof(char) * len + 1);
    if (!*result) return ERR_MEMORY_ALLOC;

    for (int i = 0; i < len; i++) {
        (*result)[i] = str[len - 1 - i];
    }
    (*result)[len] = '\0';
    return SUCCESS;
}

//флаг -u: нечетные позиции аппер
StatusCode task_u(const char *str, char **result) {
    if (!str || !result) return ERR_INVALID_ID_ARGS;

    int len = my_strlen(str);
    *result = (char*)malloc(sizeof(char) * len + 1);
    if (!*result) return ERR_MEMORY_ALLOC;

    for (int i = 0; i < len; i++) {
        if (i % 2 != 0) {
            (*result)[i] = my_toupper(str[i]);
        } else {
            (*result)[i] = str[i];
        }
    }
    (*result)[len] = '\0';
    return SUCCESS;
}


StatusCode task_n(const char *str, char **result) {
    if (!str || !result) return ERR_INVALID_ID_ARGS;

    int len = my_strlen(str);
    *result = (char*)malloc(sizeof(char) * len + 1);
    if (!*result) return ERR_MEMORY_ALLOC;

    int pos = 0;

    for (int i = 0; i < len; i++) {
        if (my_isalpha(str[i])) (*result)[pos++] = str[i];
    }
    for (int i = 0; i < len; i++) {
        if (my_isdigit(str[i])) (*result)[pos++] = str[i];
    }
    for (int i = 0; i < len; i++) {
        if (!my_isdigit(str[i]) && !my_isalpha(str[i])) (*result)[pos++] = str[i];
    }
    (*result)[len] = '\0';
    
    return SUCCESS;
}

//флаг -c: конкатенация в случайном порядке
StatusCode task_c(unsigned int seed, int count, const char **strings, char **result) {
    if (count < 0 || (count > 0 && !strings) || !result) return ERR_INVALID_ID_ARGS;
    
    int total_len = 0;
    for (int i = 0; i < count; i++) {
        total_len += my_strlen(strings[i]);
    }
    
    *result = (char*)malloc(sizeof(char) * (total_len + 1));
    if (!*result) return ERR_MEMORY_ALLOC;
    (*result)[0] = '\0'; 
    
    if (count == 0) return SUCCESS;

    int *indices = (int*)malloc(sizeof(int) * count);
    if (!indices) {
        free(*result);
        return ERR_MEMORY_ALLOC;
    }

    for (int i = 0; i < count; i++) indices[i] = i;

//алгоритм Фишера-Йетса
    srand(seed);
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    int current_pos = 0;
    for (int i = 0; i < count; i++) {
        const char *src = strings[indices[i]];
        int k = 0;
        while (src[k] != '\0') {
            (*result)[current_pos++] = src[k++];
        }
    }
    (*result)[current_pos] = '\0';
    
    free(indices);
    return SUCCESS;

}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Ошибка: недостаточно аргументов.\n");
        return 1;
    }

    const char *flag = argv[1];
    StatusCode status = SUCCESS;

    if (my_strcmp(flag, "-l")) {
        if (argc != 3) status = ERR_INVALID_ID_ARGS;
        else {
            int len = 0;
            status = task_l(argv[2], &len);
            if (status == SUCCESS) printf("Длина строки: %d\n", len);
        }
    } 
    else if (my_strcmp(flag, "-r")) {
        if (argc != 3) status = ERR_INVALID_ID_ARGS;
        else {
            char *result = NULL;
            status = task_r(argv[2], &result);
            if (status == SUCCESS) {
                printf("Перевернутая строка: %s\n", result);
                free(result);
            }
        }
    } 
    else if (my_strcmp(flag, "-u")) {
        if (argc != 3) status = ERR_INVALID_ID_ARGS;
        else {
            char *result = NULL;
            status = task_u(argv[2], &result);
            if (status == SUCCESS) {
                printf("Строка с верхним регистром: %s\n", result);
                free(result);
            }
        }
    } 
    else if (my_strcmp(flag, "-n")) {
        if (argc != 3) status = ERR_INVALID_ID_ARGS;
        else {
            char *result = NULL;
            status = task_n(argv[2], &result);
            if (status == SUCCESS) {
                printf("Отсортированная строка: %s\n", result);
                free(result);
            }
        }
    } 
    else if (my_strcmp(flag, "-c")) {
        if (argc < 3) status = ERR_INVALID_ID_ARGS; 
        else {
            unsigned int seed = 0;
            status = parse_unit(argv[2], &seed);
            if (status == SUCCESS) {
                char *result = NULL;
                status = task_c(seed, argc - 3, (const char **)(argv + 3), &result);
                if (status == SUCCESS) {
                    printf("Конкатенированная строка: %s\n", result);
                    free(result);
                }
            }
        }
    } 
    else {
        status = ERR_UNKNOWN_FLAG;
    }


    switch (status) {
        case SUCCESS:
            return 0; 
        case ERR_INVALID_ID_ARGS:
            fprintf(stderr, "Ошибка: неверное количество аргументов для данного флага.\n");
            return 1;
        case ERR_MEMORY_ALLOC:
            fprintf(stderr, "Ошибка: не удалось выделить динамическую память.\n");
            return 2;
        case ERR_INVALID_SEED:
            fprintf(stderr, "Ошибка: seed должен быть положительным целым числом.\n");
            return 3;
        case ERR_UNKNOWN_FLAG:
            fprintf(stderr, "Ошибка: неизвестный флаг. Доступны: -l, -r, -u, -n, -c.\n");
            return 4;
        default:
            fprintf(stderr, "Неизвестная системная ошибка.\n");
            return -1;
    }
}