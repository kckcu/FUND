#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// ==========================================
// 1. СТАТУС-КОДЫ И СТРУКТУРЫ
// ==========================================
typedef enum {
    SUCCESS = 0,
    ERR_ARGS,
    ERR_FILE_OPEN,
    ERR_MEMORY,
    ERR_PARSE
} StatusCode;

// Узел N-арного дерева (через First Child / Next Sibling)
typedef struct TreeNode {
    char* name;
    struct TreeNode* child;
    struct TreeNode* sibling;
} TreeNode;

// Стек для парсера (чтобы запоминать родителей при спуске в скобки)
typedef struct {
    TreeNode** data;
    int top;
    int capacity;
} Stack;

// ==========================================
// 2. РАБОТА С ПАМЯТЬЮ ДЕРЕВА И СТЕКА
// ==========================================
TreeNode* create_node(const char* name) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (!node) return NULL;
    node->name = strdup(name);
    node->child = node->sibling = NULL;
    return node;
}

void free_tree(TreeNode* root) {
    if (!root) return;
    free_tree(root->child);
    free_tree(root->sibling);
    free(root->name);
    free(root);
}

StatusCode init_stack(Stack* s, int capacity) {
    s->data = (TreeNode**)malloc(capacity * sizeof(TreeNode*));
    if (!s->data) return ERR_MEMORY;
    s->top = -1;
    s->capacity = capacity;
    return SUCCESS;
}

StatusCode push(Stack* s, TreeNode* node) {
    if (s->top >= s->capacity - 1) {
        s->capacity *= 2;
        TreeNode** temp = (TreeNode**)realloc(s->data, s->capacity * sizeof(TreeNode*));
        if (!temp) return ERR_MEMORY;
        s->data = temp;
    }
    s->data[++(s->top)] = node;
    return SUCCESS;
}

TreeNode* pop(Stack* s) {
    if (s->top < 0) return NULL;
    return s->data[(s->top)--];
}

TreeNode* peek(Stack* s) {
    if (s->top < 0) return NULL;
    return s->data[s->top];
}

void free_stack(Stack* s) {
    free(s->data);
}

// ==========================================
// 3. БИЗНЕС-ЛОГИКА: ПАРСЕР ВЫРАЖЕНИЙ
// ==========================================
StatusCode parse_expression(const char* str, TreeNode** out_root) {
    Stack stack;
    if (init_stack(&stack, 32) != SUCCESS) return ERR_MEMORY;

    *out_root = NULL;
    TreeNode* curr = NULL;
    bool is_first_child = false;
    int i = 0;

    while (str[i] != '\0') {
        if (isspace(str[i])) { i++; continue; }

        if (str[i] == '(') {
            push(&stack, curr);
            is_first_child = true;
            i++;
        } else if (str[i] == ',') {
            i++;
        } else if (str[i] == ')') {
            curr = pop(&stack); // Возвращаемся к родителю
            if (!curr) { free_stack(&stack); return ERR_PARSE; } // Несоответствие скобок
            i++;
        } else {
            // Читаем имя узла (до пробела, запятой или скобок)
            char name[256];
            int len = 0;
            while (str[i] != '\0' && !isspace(str[i]) && 
                   str[i] != '(' && str[i] != ')' && str[i] != ',') {
                if (len < 255) name[len++] = str[i];
                i++;
            }
            name[len] = '\0';

            if (len > 0) {
                TreeNode* new_node = create_node(name);
                if (!new_node) { free_stack(&stack); return ERR_MEMORY; }

                if (!*out_root) {
                    *out_root = new_node;
                } else if (is_first_child) {
                    TreeNode* parent = peek(&stack);
                    if (parent) parent->child = new_node;
                    is_first_child = false;
                } else {
                    if (curr) curr->sibling = new_node;
                }
                curr = new_node;
            }
        }
    }
    free_stack(&stack);
    return SUCCESS;
}

// ==========================================
// 4. ФУНКЦИИ ВЫВОДА ДАННЫХ
// ==========================================
void print_tree(FILE* out, TreeNode* root, int depth) {
    if (!root) return;
    
    // Печатаем отступы для текущего узла
    for (int i = 0; i < depth; i++) fprintf(out, "    ");
    fprintf(out, "%s\n", root->name);
    
    // Рекурсивно печатаем первого ребенка (с увеличением глубины)
    print_tree(out, root->child, depth + 1);
    
    // Рекурсивно печатаем братьев (на той же глубине)
    print_tree(out, root->sibling, depth);
}

// ==========================================
// 5. РАБОТА С ФАЙЛАМИ (Начальник)
// ==========================================
StatusCode process_files(const char* in_path, const char* out_path) {
    FILE* fin = fopen(in_path, "r");
    if (!fin) return ERR_FILE_OPEN;

    FILE* fout = fopen(out_path, "w");
    if (!fout) { fclose(fin); return ERR_FILE_OPEN; }

    char line[1024];
    int expr_count = 1;

    while (fgets(line, sizeof(line), fin)) {
        // Убираем символ переноса строки
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        TreeNode* root = NULL;
        if (parse_expression(line, &root) == SUCCESS && root != NULL) {
            fprintf(fout, "=== Дерево выражения %d ===\n", expr_count++);
            fprintf(fout, "Исходная строка: %s\n\n", line);
            print_tree(fout, root, 0);
            fprintf(fout, "\n---------------------------\n\n");
            free_tree(root);
        } else {
            fprintf(fout, "[ОШИБКА] Некорректное выражение: %s\n", line);
            free_tree(root);
        }
    }

    fclose(fin);
    fclose(fout);
    return SUCCESS;
}

int main(int argc, char* argv[]) {
    // Валидация аргументов командной строки
    if (argc != 3) {
        printf("Использование: %s <входной_файл> <выходной_файл>\n", argv[0]);
        return 1;
    }

    StatusCode status = process_files(argv[1], argv[2]);

    switch (status) {
        case SUCCESS:
            printf("Успешно! Деревья записаны в файл '%s'.\n", argv[2]);
            return 0;
        case ERR_FILE_OPEN:
            printf("Ошибка: Не удалось открыть один из файлов.\n");
            return 1;
        case ERR_MEMORY:
            printf("Ошибка: Нехватка динамической памяти.\n");
            return 1;
        default:
            printf("Неизвестная ошибка.\n");
            return 1;
    }
}