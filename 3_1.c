#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Перечислимый тип для статус-кодов ошибок[cite: 10] */
typedef enum {
    SUCCESS = 0,
    ERR_INVALID_R,
    ERR_MEMORY_ALLOC,
    ERR_INVALID_ARGS
} StatusCode;

/* Реализация арифметического сложения исключительно через битовые операции[cite: 11] */
int bit_add(int a, int b) {
    int carry;
    while (b != 0) {
        carry = (a & b) << 1;
        a = a ^ b;
        b = carry;
    }
    return a;
}

/* Реализация арифметического вычитания через битовые операции[cite: 11] */
int bit_sub(int a, int b) {
    /* a - b = a + (~b + 1) */
    return bit_add(a, bit_add(~b, 1));
}

/*
 * Функция перевода числа из десятичной системы в систему с основанием 2^r.[cite: 11]
 * - num: исходное число (ограничено const[cite: 10])
 * - r: степень двойки (ограничено const[cite: 10])
 * - result: указатель для возврата результата из функции[cite: 10]
 * Возвращает код возврата StatusCode[cite: 10]
 */
StatusCode convert_to_base_2r(const int num, const int r, char **result) {
    /* Валидация параметра r (должно быть 1 <= r <= 5)[cite: 11] */
    if (r < 1 || r > 5) {
        return ERR_INVALID_R;
    }

    /* Выделение памяти для строки: максимум 32 бита + знак минус + терминатор \0 = 34 байта[cite: 10] */
    char *buffer = (char *)malloc(34 * sizeof(char));
    if (!buffer) {
        return ERR_MEMORY_ALLOC;
    }

    if (num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        *result = buffer;
        return SUCCESS;
    }

    unsigned int unum;
    bool is_negative = false;
    
    if (num < 0) {
        is_negative = true;
        /* unum = abs(num) = ~num + 1 через битовые операции[cite: 11] */
        unum = bit_add(~num, 1);
    } else {
        unum = num;
    }

    /* Создаем маску для остатка от деления на 2^r[cite: 11] */
    /* mask = (1 << r) - 1 */
    int mask = bit_sub(1 << r, 1);
    int index = 0;
    
    /* Доступный алфавит для систем счисления вплоть до 32-ричной (2^5 = 32) */
    const char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

    while (unum > 0) {
        int rem = unum & mask; 
        buffer[index] = alphabet[rem]; /* Операции обращения к памяти[cite: 11] */
        index = bit_add(index, 1); /* Использование битового сложения вместо index++[cite: 11] */
        unum >>= r; /* Использование битового сдвига вместо деления[cite: 11] */
    }

    if (is_negative) {
        buffer[index] = '-';
        index = bit_add(index, 1);
    }

    buffer[index] = '\0';

    /* Переворот строки для правильного порядка символов, используем битовую арифметику[cite: 11] */
    int left = 0;
    int right = bit_sub(index, 1);
    while (left < right) {
        char temp = buffer[left];
        buffer[left] = buffer[right];
        buffer[right] = temp;
        
        left = bit_add(left, 1);
        right = bit_sub(right, 1);
    }

    *result = buffer;
    return SUCCESS;
}

int main(int argc, char *argv[]) {
    /* Валидация количества аргументов при подаче их через командную строку[cite: 10] */
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <число> <степень_r>\n", argv[0]);
        return ERR_INVALID_ARGS;
    }

    char *endptr;
    
    /* Валидация конвертации строки в число[cite: 10] */
    long num_long = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Ошибка: неверный формат аргумента <число>.\n");
        return ERR_INVALID_ARGS;
    }

    long r_long = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Ошибка: неверный формат аргумента <степень_r>.\n");
        return ERR_INVALID_ARGS;
    }

    char *result_str = NULL;
    
    /* Вызов функции с обработкой ошибок через switch/case[cite: 10] */
    StatusCode status = convert_to_base_2r((int)num_long, (int)r_long, &result_str);

    switch (status) {
        case SUCCESS:
            printf("%s\n", result_str);
            free(result_str); /* Корректное освобождение динамической памяти[cite: 10] */
            break;
        case ERR_INVALID_R:
            fprintf(stderr, "Ошибка: значение r должно быть в диапазоне от 1 до 5[cite: 11].\n");
            break;
        case ERR_MEMORY_ALLOC:
            fprintf(stderr, "Ошибка: не удалось выделить память[cite: 10].\n");
            break;
        default:
            fprintf(stderr, "Неизвестная ошибка.\n");
            break;
    }

    return status;
}