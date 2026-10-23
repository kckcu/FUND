#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// ==========================================
// 1. СТАТУС-КОДЫ И СТРУКТУРЫ
// ==========================================
typedef enum {
    SUCCESS = 0,
    ERR_MEMORY,
    ERR_FILE_OPEN,
    ERR_PARSE
} StatusCode;

// Узел односвязного списка для хранения одного члена многочлена
typedef struct PolyNode {
    int coeff;
    int degree;
    struct PolyNode* next;
} PolyNode;

// ==========================================
// 2. БАЗОВЫЕ ОПЕРАЦИИ С МНОГОЧЛЕНАМИ
// ==========================================

// Создание нового узла
PolyNode* create_node(int coeff, int degree) {
    PolyNode* node = (PolyNode*)malloc(sizeof(PolyNode));
    if (node) {
        node->coeff = coeff;
        node->degree = degree;
        node->next = NULL;
    }
    return node;
}

// Очистка памяти многочлена
void free_poly(PolyNode* head) {
    while (head) {
        PolyNode* temp = head;
        head = head->next;
        free(temp);
    }
}

// Вставка члена в многочлен с сохранением сортировки по убыванию степени
StatusCode insert_term(PolyNode** head, int coeff, int degree) {
    if (coeff == 0) return SUCCESS; 

    PolyNode* new_node = create_node(coeff, degree);
    if (!new_node) return ERR_MEMORY;

    if (!*head || (*head)->degree < degree) {
        new_node->next = *head;
        *head = new_node;
        return SUCCESS;
    }

    PolyNode* curr = *head;
    while (curr->next && curr->next->degree >= degree) {
        curr = curr->next;
    }

    if (curr->degree == degree) {
        curr->coeff += coeff;
        free(new_node);
    } else {
        new_node->next = curr->next;
        curr->next = new_node;
    }
    return SUCCESS;
}

// Сложение двух многочленов
PolyNode* poly_add(const PolyNode* p1, const PolyNode* p2) {
    PolyNode* result = NULL;
    while (p1) { insert_term(&result, p1->coeff, p1->degree); p1 = p1->next; }
    while (p2) { insert_term(&result, p2->coeff, p2->degree); p2 = p2->next; }
    return result;
}

// Вычисление многочлена в точке
long long poly_eval(const PolyNode* p, int x) {
    long long result = 0;
    while (p) {
        result += p->coeff * pow(x, p->degree);
        p = p->next;
    }
    return result;
}

// Вывод многочлена (Исправлено условие вывода плюса)
void print_poly(const PolyNode* head) {
    if (!head) { printf("0\n"); return; }
    const PolyNode* p = head;
    while (p) {
        if (p->coeff > 0 && p != head) printf("+");
        printf("%dx^%d ", p->coeff, p->degree);
        p = p->next;
    }
    printf("\n");
}

// ==========================================
// 3. ПАРСИНГ ФАЙЛА И ИНТЕРПРЕТАТОР
// ==========================================

PolyNode* parse_poly_string(const char* str) {
    PolyNode* p = NULL;
    // Заглушка парсера для примера: добавляет 2x^2
    insert_term(&p, 2, 2);
    return p;
}

StatusCode process_script(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return ERR_FILE_OPEN;

    PolyNode* accumulator = NULL; // Сумматор
    char line[256];
    int in_multiline_comment = 0;

    printf("=== Выполнение скрипта ===\n");

    while (fgets(line, sizeof(line), file)) {
        char* ptr = line;

        while (*ptr == ' ' || *ptr == '\t') ptr++; 
        
        if (in_multiline_comment) {
            if (strchr(ptr, ']')) in_multiline_comment = 0;
            continue;
        }
        
        if (*ptr == '[') {
            in_multiline_comment = 1;
            continue;
        }
        if (*ptr == '%') continue;

        if (strncmp(ptr, "$Add(", 5) == 0) {
            printf("Выполняется сложение (Add)...\n");
            PolyNode* p1 = parse_poly_string("dummy");
            PolyNode* temp = accumulator;
            accumulator = poly_add(accumulator, p1);
            free_poly(temp);
            free_poly(p1);
            
            printf("Результат в сумматоре: ");
            print_poly(accumulator);
        }
        else if (strncmp(ptr, "Eval(", 5) == 0) {
            printf("Вычисляется Eval(1) для сумматора...\n");
            long long res = poly_eval(accumulator, 1);
            printf("Результат вычисления: %lld\n", res);
        }
    }

    free_poly(accumulator);
    fclose(file);
    return SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Использование: %s <файл_инструкций>\n", argv[0]);
        return 1;
    }

    if (process_script(argv[1]) != SUCCESS) {
        printf("Ошибка обработки файла.\n");
        return 1;
    }

    return 0;
}
