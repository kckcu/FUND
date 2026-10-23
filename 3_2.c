#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Перечислимый тип для статус-кодов ошибок[cite: 10] */
typedef enum {
    SUCCESS = 0,
    ERR_INVALID_DATA,
    ERR_MEMORY_ALLOC,
    ERR_IO
} StatusCode;

/* Структура для описания заявки на доклад */
typedef struct {
    int id;
    int start_time;  /* Время начала в минутах от 00:00 */
    int finish_time; /* Время окончания в минутах от 00:00 */
} Report;

/* Компаратор для сортировки докладов по времени окончания (и по времени начала при совпадении) */
int compare_reports(const void *a, const void *b) {
    const Report *ra = (const Report *)a;
    const Report *rb = (const Report *)b;
    
    if (ra->finish_time != rb->finish_time) {
        return ra->finish_time - rb->finish_time;
    }
    return ra->start_time - rb->start_time;
}

/*
 * Функция вычисления максимального количества докладов[cite: 10]
 * - reports: массив заявок (ограничен const для защиты от модификации[cite: 10])
 * - n: количество заявок (ограничено const[cite: 10])
 * - k: количество доступных залов (ограничено const[cite: 10])
 * - max_count: указатель для возврата результата[cite: 10]
 */
StatusCode calculate_max_reports(const Report *reports, const int n, const int k, int *max_count) {
    /* Валидация аргументов[cite: 10] */
    if (!reports || n <= 0 || k <= 0 || !max_count) {
        return ERR_INVALID_DATA;
    }

    /* Выделение памяти под копию массива (так как исходный массив изменять нельзя)[cite: 10] */
    Report *sorted_reports = (Report *)malloc(n * sizeof(Report));
    if (!sorted_reports) {
        return ERR_MEMORY_ALLOC;
    }
    
    /* Копируем данные для дальнейшей сортировки */
    memcpy(sorted_reports, reports, n * sizeof(Report));
    qsort(sorted_reports, n, sizeof(Report), compare_reports);

    /* Массив для хранения времени окончания последнего доклада в каждом зале */
    int *halls = (int *)calloc(k, sizeof(int));
    if (!halls) {
        free(sorted_reports);
        return ERR_MEMORY_ALLOC;
    }

    int count = 0;

    for (int i = 0; i < n; ++i) {
        int best_hall = -1;
        int max_finish = -1;

        /* Ищем зал, который уже освободился к моменту start_time, 
           при этом выбираем тот, в котором простой будет минимальным (жадный алгоритм) */
        for (int j = 0; j < k; ++j) {
            if (halls[j] <= sorted_reports[i].start_time) {
                if (halls[j] > max_finish) {
                    max_finish = halls[j];
                    best_hall = j;
                }
            }
        }

        /* Если подходящий зал найден, занимаем его */
        if (best_hall != -1) {
            halls[best_hall] = sorted_reports[i].finish_time;
            count++;
        }
    }

    *max_count = count;

    /* Корректное освобождение всей динамически выделенной памяти[cite: 10] */
    free(halls);
    free(sorted_reports);
    
    return SUCCESS;
}

/* Вспомогательная функция для валидации и парсинга времени в формате HH:MM[cite: 10] */
StatusCode parse_time(const char *time_str, int *minutes) {
    int hh, mm;
    if (sscanf(time_str, "%d:%d", &hh, &mm) != 2) {
        return ERR_INVALID_DATA;
    }
    /* Часы задаются в диапазоне от 00 до 23, минуты от 00 до 59[cite: 10] */
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59) {
        return ERR_INVALID_DATA;
    }
    *minutes = hh * 60 + mm;
    return SUCCESS;
}

int main() {
    int n, k;
    
    /* Считывание первой строки: n и k[cite: 10] */
    if (scanf("%d %d", &n, &k) != 2) {
        fprintf(stderr, "Ошибка чтения входных данных (ожидаются n и k).\n");
        return ERR_IO;
    }

    /* n и k должны быть положительными[cite: 10] */
    if (n <= 0 || k <= 0) {
        fprintf(stderr, "Количество заявок и залов должны быть положительными целыми числами[cite: 10].\n");
        return ERR_INVALID_DATA;
    }

    Report *reports = (Report *)malloc(n * sizeof(Report));
    if (!reports) {
        fprintf(stderr, "Ошибка выделения памяти для заявок[cite: 10].\n");
        return ERR_MEMORY_ALLOC;
    }

    /* Считывание заявок */
    for (int i = 0; i < n; ++i) {
        int id;
        char start_str[10], finish_str[10];
        
        /* Формат: id start_time finish_time[cite: 10] */
        if (scanf("%d %9s %9s", &id, start_str, finish_str) != 3) {
            fprintf(stderr, "Ошибка чтения данных заявки на строке %d.\n", i + 1);
            free(reports);
            return ERR_IO;
        }

        /* id уникальный положительный целочисленный идентификатор[cite: 10] */
        if (id <= 0) {
            fprintf(stderr, "ID доклада должен быть положительным (ошибка на строке %d)[cite: 10].\n", i + 1);
            free(reports);
            return ERR_INVALID_DATA;
        }

        int start_min, finish_min;
        if (parse_time(start_str, &start_min) != SUCCESS || parse_time(finish_str, &finish_min) != SUCCESS) {
            fprintf(stderr, "Неверный формат времени (ошибка на строке %d). Используйте HH:MM[cite: 10].\n", i + 1);
            free(reports);
            return ERR_INVALID_DATA;
        }

        /* Время начала каждого доклада должно быть строго меньше времени его окончания[cite: 10] */
        if (start_min >= finish_min) {
            fprintf(stderr, "Время начала должно быть строго меньше времени окончания (ошибка на строке %d)[cite: 10].\n", i + 1);
            free(reports);
            return ERR_INVALID_DATA;
        }

        reports[i].id = id;
        reports[i].start_time = start_min;
        reports[i].finish_time = finish_min;
    }

    int max_reports = 0;
    StatusCode status = calculate_max_reports(reports, n, k, &max_reports);

    /* Обработка статус-кодов вызываемой функции через switch/case[cite: 10] */
    switch (status) {
        case SUCCESS:
            /* Вывод одного целого числа в стандартный поток вывода[cite: 10] */
            printf("%d\n", max_reports);
            break;
        case ERR_INVALID_DATA:
            fprintf(stderr, "Ошибка: переданы некорректные данные.\n");
            break;
        case ERR_MEMORY_ALLOC:
            fprintf(stderr, "Ошибка: невозможно выделить память[cite: 10].\n");
            break;
        default:
            fprintf(stderr, "Произошла неизвестная ошибка.\n");
            break;
    }

    free(reports);
    return status;
}