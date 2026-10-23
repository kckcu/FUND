#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>



typedef enum {
    SUCCESS,
    ERR_INVALID_ARGC,
    ERR_INVALID_FLAG,
    ERR_INVALID_NUMBER,
    ERR_OUT_OF_BOUNDS,
    ERR_OVERFLOW,
    ERR_MEMORY_ALLOCATION
} StatusCode;

//Проверяем что число целое
StatusCode parse_long_long(const char* str, long long* result) {
    if (str == NULL || *str == '\0') return ERR_INVALID_NUMBER;
    char* endptr;
    *result = strtoll(str, &endptr, 10);
    if (*endptr != '\0') return ERR_INVALID_NUMBER;
    return SUCCESS;
}

// -h: кратные числа
StatusCode get_multiples(const long long x, long long** results, int* count) {
    if (x == 0) return ERR_INVALID_NUMBER;
    *count = 0;
    long long abs_x = llabs(x);
    if (abs_x > 100) return SUCCESS; 

    *results = (long long*)malloc((100 / abs_x) * sizeof(long long));
    if (!*results) return ERR_MEMORY_ALLOCATION;

    for (long long i = abs_x; i <= 100; i += abs_x) {
        (*results)[(*count)++] = i;
    }
    return SUCCESS;
}

// -p: проверка на простоту 
StatusCode check_prime(const long long x, bool* is_prime, bool* is_composite) {
    *is_prime = false;
    *is_composite = false;
    if (x <= 1) return SUCCESS; 
    
    *is_prime = true;
    for (long long i = 2; i * i <= x; ++i) {
        if (x % i == 0) {
            *is_prime = false;
            *is_composite = true;
            break;
        }
    }
    return SUCCESS;
}

// -s: разделение на 16-ричные цифры 
StatusCode split_hex(const long long x, char** result_str) {
    long long abs_x = llabs(x);
    char buffer[100];
    sprintf(buffer, "%llX", (unsigned long long)abs_x);
    
    int len = strlen(buffer);
    *result_str = (char*)malloc((len * 2 + 1) * sizeof(char));
    if (!*result_str) return ERR_MEMORY_ALLOCATION;
    
    int pos = 0;
    for (int i = 0; i < len; ++i) {
        (*result_str)[pos++] = buffer[i];
        if (i < len - 1) (*result_str)[pos++] = ' ';
    }
    (*result_str)[pos] = '\0';
    return SUCCESS;
}

// -e: таблица степеней
StatusCode get_powers(const long long x, long long*** table) {
    if (x < 1 || x > 10) return ERR_OUT_OF_BOUNDS;
    *table = (long long**)malloc(10 * sizeof(long long*));
    if (!*table) return ERR_MEMORY_ALLOCATION;

    for (int base = 1; base <= 10; ++base) {
        (*table)[base - 1] = (long long*)malloc(x * sizeof(long long));
        
        if (!(*table)[base - 1]) {
            for (int i = 0; i < base - 1; ++i) {
                free((*table)[i]);
            }
            free(*table);
            return ERR_MEMORY_ALLOCATION;
        }

        long long current_val = 1;
        for (long long exp = 1; exp <= x; ++exp) {
            current_val *= base;
            (*table)[base - 1][exp - 1] = current_val;
        }
    }
    return SUCCESS;
}

// -a: сумма чисел от 1 до x 
StatusCode sum_to_x(const long long x, long long* sum) {
    if (x < 1) return ERR_INVALID_NUMBER;
    if (x > UINT_MAX) return ERR_OVERFLOW; 
    *sum = (x * (x + 1)) / 2;
    return SUCCESS;
}

// -f: факториал x 
StatusCode factorial(const long long x, unsigned long long* fact) {
    if (x < 0) return ERR_INVALID_NUMBER;
    if (x > 20) return ERR_OVERFLOW; 
    
    *fact = 1;
    for (long long i = 2; i <= x; ++i) {
        *fact *= i;
    }
    return SUCCESS;
}


void print_error(StatusCode code) {
    switch (code) {
        case ERR_INVALID_ARGC: puts("Ошибка: неверное количество аргументов."); break;
        case ERR_INVALID_FLAG: puts("Ошибка: неизвестный флаг."); break;
        case ERR_INVALID_NUMBER: puts("Ошибка: некорректное число."); break;
        case ERR_OUT_OF_BOUNDS: puts("Ошибка: число выходит за допустимые пределы."); break;
        case ERR_OVERFLOW: puts("Ошибка: переполнение при вычислениях."); break;
        case ERR_MEMORY_ALLOCATION: puts("Ошибка: не удалось выделить память."); break;
        default: puts("Неизвестная ошибка."); break;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        print_error(ERR_INVALID_ARGC);
        return ERR_INVALID_ARGC;
    }

    long long x;
    StatusCode parse_status = parse_long_long(argv[1], &x);
    if (parse_status != SUCCESS) {
        print_error(parse_status);
        return parse_status;
    }

    const char* flag = argv[2];
    if ((flag[0] != '-') || strlen(flag) != 2) {
        print_error(ERR_INVALID_FLAG);
        return ERR_INVALID_FLAG;
    }

    char action = flag[1];
    StatusCode process_status = SUCCESS;

    switch (action) {
        case 'h': {
            long long* multiples = NULL;
            int count = 0;
            process_status = get_multiples(x, &multiples, &count);
            if (process_status == SUCCESS) {
                if (count == 0) {
                    puts("Нет кратных чисел в пределах 100");
                } else {
                    for (int i = 0; i < count; ++i) {
                        printf("%lld ", multiples[i]);
                    }
                    putchar('\n');
                }
                free(multiples);
            }
            break;
        }
        case 'p': {
            bool is_prime, is_composite;
            process_status = check_prime(x, &is_prime, &is_composite);
            if (process_status == SUCCESS) {
                if (is_prime) puts("Число является простым.");
                else if (is_composite) puts("Число является составным.");
                else puts("Число не является ни простым, ни составным (0, 1 или отрицательное).");
            }
            break;
        }
        case 's': {
            char* hex_str = NULL;
            process_status = split_hex(x, &hex_str);
            if (process_status == SUCCESS) {
                printf("%s\n", hex_str);
                free(hex_str);
            }
            break;
        }
       case 'e': {
            long long** table = NULL; 
            process_status = get_powers(x, &table);
            
            if (process_status == SUCCESS) {
                for (int base = 1; base <= 10; ++base) {
                    for (long long exp = 1; exp <= x; ++exp) {
                        printf("%lld ", table[base - 1][exp - 1]);
                    }
                    putchar('\n');
  
                    free(table[base - 1]);
                }
                free(table);
            }
            break;
        }
        case 'a': {
            long long sum = 0;
            process_status = sum_to_x(x, &sum);
            if (process_status == SUCCESS) printf("%lld\n", sum);
            break;
        }
        case 'f': {
            unsigned long long fact = 0;
            process_status = factorial(x, &fact);
            if (process_status == SUCCESS) printf("%llu\n", fact);
            break;
        }
        default:
            process_status = ERR_INVALID_FLAG;
    }

    if (process_status != SUCCESS) {
        print_error(process_status);
        return process_status;
    }

    return SUCCESS;
}
