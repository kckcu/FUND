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
    ERR_MEMORY
} StatusCode;

typedef struct TreeNode {
    char* word;
    int count;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// ==========================================
// 2. РАБОТА С БИНАРНЫМ ДЕРЕВОМ ПОИСКА
// ==========================================

TreeNode* create_node(const char* word) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node) {
        node->word = strdup(word);
        node->count = 1;
        node->left = node->right = NULL;
    }
    return node;
}

void free_tree(TreeNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root->word);
    free(root);
}

StatusCode insert_word(TreeNode** root, const char* word) {
    if (!*root) {
        *root = create_node(word);
        return *root ? SUCCESS : ERR_MEMORY;
    }
    int cmp = strcmp(word, (*root)->word);
    if (cmp == 0) {
        (*root)->count++;
    } else if (cmp < 0) {
        return insert_word(&((*root)->left), word);
    } else {
        return insert_word(&((*root)->right), word);
    }
    return SUCCESS;
}

int get_word_count(TreeNode* root, const char* word) {
    if (!root) return 0;
    int cmp = strcmp(word, root->word);
    if (cmp == 0) return root->count;
    if (cmp < 0) return get_word_count(root->left, word);
    return get_word_count(root->right, word);
}

int get_tree_depth(TreeNode* root) {
    if (!root) return 0;
    int left_d = get_tree_depth(root->left);
    int right_d = get_tree_depth(root->right);
    return (left_d > right_d ? left_d : right_d) + 1;
}

void find_min_max_len(TreeNode* root, TreeNode** min_node, TreeNode** max_node) {
    if (!root) return;
    
    if (!*min_node || strlen(root->word) < strlen((*min_node)->word)) {
        *min_node = root;
    }
    if (!*max_node || strlen(root->word) > strlen((*max_node)->word)) {
        *max_node = root;
    }
    
    find_min_max_len(root->left, min_node, max_node);
    find_min_max_len(root->right, min_node, max_node);
}

// --- Функции для вывода ТОП-N слов ---
void collect_nodes(TreeNode* root, TreeNode*** arr, int* index) {
    if (!root) return;
    (*arr)[(*index)++] = root;
    collect_nodes(root->left, arr, index);
    collect_nodes(root->right, arr, index);
}

int count_nodes(TreeNode* root) {
    if (!root) return 0;
    return 1 + count_nodes(root->left) + count_nodes(root->right);
}

int cmp_freq(const void* a, const void* b) {
    TreeNode* n1 = *(TreeNode**)a;
    TreeNode* n2 = *(TreeNode**)b;
    return n2->count - n1->count; 
}

void print_top_n(TreeNode* root, int n) {
    int total = count_nodes(root);
    if (total == 0) {
        printf("Дерево пусто.\n");
        return;
    }
    
    TreeNode** arr = (TreeNode**)malloc(total * sizeof(TreeNode*));
    int index = 0;
    collect_nodes(root, &arr, &index);
    
    qsort(arr, total, sizeof(TreeNode*), cmp_freq);
    
    printf("--- ТОП-%d частых слов ---\n", n);
    for (int i = 0; i < n && i < total; i++) {
        printf("%d. %s (Встречается: %d раз)\n", i + 1, arr[i]->word, arr[i]->count);
    }
    free(arr);
}

// ==========================================
// 3. СОХРАНЕНИЕ И ВОССТАНОВЛЕНИЕ ДЕРЕВА
// ==========================================

void serialize_tree(FILE* f, TreeNode* root) {
    if (!root) {
        fprintf(f, "#\n");
        return;
    }
    fprintf(f, "%s %d\n", root->word, root->count);
    serialize_tree(f, root->left);
    serialize_tree(f, root->right);
}

TreeNode* deserialize_tree(FILE* f) {
    char buffer[512];
    if (fscanf(f, "%511s", buffer) != 1) return NULL;
    
    if (strcmp(buffer, "#") == 0) return NULL;
    
    int count = 0;
    fscanf(f, "%d", &count);
    
    TreeNode* node = create_node(buffer);
    node->count = count;
    node->left = deserialize_tree(f);
    node->right = deserialize_tree(f);
    
    return node;
}

// ==========================================
// 4. ПАРСИНГ ФАЙЛА И МЕНЮ
// ==========================================

bool is_separator(char c, const char* seps) {
    if (isspace(c)) return true;
    for (int i = 0; seps[i]; i++) {
        if (c == seps[i]) return true;
    }
    return false;
}

StatusCode parse_file_to_tree(const char* filename, const char* seps, TreeNode** root) {
    FILE* f = fopen(filename, "r");
    if (!f) return ERR_FILE_OPEN;

    char buffer[1024];
    int idx = 0;
    int ch;

    while ((ch = fgetc(f)) != EOF) {
        if (is_separator(ch, seps)) {
            if (idx > 0) {
                buffer[idx] = '\0';
                insert_word(root, buffer);
                idx = 0;
            }
        } else {
            if (idx < 1023) buffer[idx++] = (char)ch;
        }
    }
    if (idx > 0) {
        buffer[idx] = '\0';
        insert_word(root, buffer);
    }
    
    fclose(f);
    return SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Использование: %s <путь_к_файлу> [сепаратор1] [сепаратор2] ...\n", argv[0]);
        return 1;
    }

    char custom_seps[256] = {0};
    int sep_idx = 0;
    for (int i = 2; i < argc; i++) {
        if (argv[i][0] != '\0' && sep_idx < 255) {
            custom_seps[sep_idx++] = argv[i][0];
        }
    }
    custom_seps[sep_idx] = '\0';

    TreeNode* root = NULL;
    if (parse_file_to_tree(argv[1], custom_seps, &root) != SUCCESS) {
        printf("Ошибка открытия файла текста.\n");
        return 1;
    }

    printf("=== ДЕРЕВО УСПЕШНО ПОСТРОЕНО ===\n");

    int choice;
    char buffer[512];
    
    while (1) {
        printf("\n1. Найти частоту слова\n2. Топ N частых слов\n3. Самое длинное и короткое слово\n");
        printf("4. Глубина дерева\n5. Сохранить дерево в файл\n6. Восстановить дерево из файла\n7. Выход\n> ");
        
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            printf("Введите слово: ");
            scanf("%511s", buffer);
            int cnt = get_word_count(root, buffer);
            printf("Слово '%s' встречается %d раз(а).\n", buffer, cnt);
        }
        else if (choice == 2) {
            int n;
            printf("Введите N: ");
            scanf("%d", &n);
            print_top_n(root, n);
        }
        else if (choice == 3) {
            TreeNode *min_n = NULL, *max_n = NULL;
            find_min_max_len(root, &min_n, &max_n);
            if (min_n && max_n) {
                printf("Самое короткое: '%s'\n", min_n->word);
                printf("Самое длинное: '%s'\n", max_n->word);
            }
        }
        else if (choice == 4) {
            printf("Глубина дерева: %d\n", get_tree_depth(root));
        }
        else if (choice == 5) {
            FILE* f = fopen("tree_backup.txt", "w");
            if (f) {
                serialize_tree(f, root);
                fclose(f);
                printf("Дерево сохранено в 'tree_backup.txt'.\n");
            }
        }
        else if (choice == 6) {
            FILE* f = fopen("tree_backup.txt", "r");
            if (f) {
                free_tree(root);
                root = deserialize_tree(f);
                fclose(f);
                printf("Дерево восстановлено из файла! Его структура полностью сохранена.\n");
            } else {
                printf("Файл 'tree_backup.txt' не найден.\n");
            }
        }
        else if (choice == 7) {
            break;
        }
    }

    free_tree(root);
    printf("Память очищена. Программа завершена.\n");
    return 0;
}