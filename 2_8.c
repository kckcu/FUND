#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// Указатель на функцию сложения
typedef char* (*AdderFunction)(const char*, const char*, int);

// Перевод символа в число ('A' -> 10)
int char_to_val(char c) {
    if (isdigit(c)) return c - '0';
    if (isalpha(c)) return toupper(c) - 'A' + 10;
    return 0;
}

// Перевод числа в символ (10 -> 'A')
char val_to_char(int v) {
    if (v >= 0 && v <= 9) return v + '0';
    return v - 10 + 'A';
}

// Переворот строки
void reverse_string(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

// Работник: сложение в столбик
char* add_big_numbers(const char* num1, const char* num2, int base) {
    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int max_len = len1 > len2 ? len1 : len2;
    
    char* result = (char*)malloc(max_len + 2);
    if (!result) return NULL;

    int carry = 0;
    int i = len1 - 1;
    int j = len2 - 1;
    int res_idx = 0;

    while (i >= 0 || j >= 0 || carry > 0) {
        int digit1 = (i >= 0) ? char_to_val(num1[i]) : 0;
        int digit2 = (j >= 0) ? char_to_val(num2[j]) : 0;

        int sum = digit1 + digit2 + carry;
        carry = sum / base;
        int new_digit = sum % base;

        result[res_idx++] = val_to_char(new_digit);
        i--;
        j--;
    }
    result[res_idx] = '\0';

    reverse_string(result);

    // Удаление ведущих нулей
    int lead_zeros = 0;
    while (result[lead_zeros] == '0' && result[lead_zeros + 1] != '\0') {
        lead_zeros++;
    }
    if (lead_zeros > 0) {
        memmove(result, result + lead_zeros, strlen(result) - lead_zeros + 1);
    }

    return result;
}

// Начальник: переменное число аргументов
char* sum_all_numbers(AdderFunction adder, int base, int count, ...) {
    if (count <= 0) return NULL;

    va_list args;
    va_start(args, count);

    const char* first = va_arg(args, const char*);
    char* current_sum = (char*)malloc(strlen(first) + 1);
    strcpy(current_sum, first);

    for (int i = 1; i < count; i++) {
        const char* next_num = va_arg(args, const char*);
        char* temp_sum = adder(current_sum, next_num, base); 
        free(current_sum);
        current_sum = temp_sum;
    }

    va_end(args);
    return current_sum;
}

// Демонстрация (тесты)
int main() {
    char* res1 = sum_all_numbers(add_big_numbers, 10, 2, "99999999999999999999", "1");
    printf("База 10: 99999999999999999999 + 1 = %s\n", res1);
    free(res1);

    char* res2 = sum_all_numbers(add_big_numbers, 16, 2, "FF", "1");
    printf("База 16: FF + 1 = %s\n", res2);
    free(res2);

    char* res3 = sum_all_numbers(add_big_numbers, 2, 3, "1010", "1100", "111");
    printf("База  2: 1010 + 1100 + 111 = %s\n", res3);
    free(res3);

    char* res4 = sum_all_numbers(add_big_numbers, 10, 2, "000045", "005");
    printf("База 10 (удаление нулей): 000045 + 005 = %s\n", res4);
    free(res4);

    return 0;
}