#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1. Определение статус-кодов для обработки ошибок[cite: 19]
typedef enum {
    SUCCESS = 0,
    ERR_INVALID_ARGS,
    ERR_FILE_OPEN,
    ERR_MEMORY_ALLOC,
    ERR_EMPTY_FILE
} StatusCode;

// 2. Структуры данных
// Узел дерева Хаффмана
typedef struct HuffNode {
    unsigned char byte_val;
    unsigned int freq;
    struct HuffNode *left, *right;
} HuffNode;

// Минимальная бинарная куча (Min-Heap)
typedef struct {
    HuffNode **array;
    int size;
    int capacity;
} MinHeap;

// Структура для хранения итогового кода байта
typedef struct {
    char code[256];
    unsigned int freq;
    int is_present;
} HuffCode;

// Структура для возврата итоговой статистики[cite: 21]
typedef struct {
    unsigned long original_bytes;
    unsigned long encoded_bits;
    unsigned long encoded_bytes;
    double savings_percent;
} CompressStats;

// --- Функции для работы с узлами и кучей ---

HuffNode* create_node(unsigned char byte_val, unsigned int freq) {
    HuffNode* node = (HuffNode*)malloc(sizeof(HuffNode));
    if (!node) return NULL;
    node->byte_val = byte_val;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

MinHeap* create_min_heap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) return NULL;
    heap->size = 0;
    heap->capacity = capacity;
    heap->array = (HuffNode**)malloc(capacity * sizeof(HuffNode*));
    if (!heap->array) {
        free(heap);
        return NULL;
    }
    return heap;
}

void swap_nodes(HuffNode** a, HuffNode** b) {
    HuffNode* t = *a;
    *a = *b;
    *b = t;
}

// Восстановление свойства минимальной кучи вниз
void min_heapify(MinHeap* heap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->array[left]->freq < heap->array[smallest]->freq)
        smallest = left;
    if (right < heap->size && heap->array[right]->freq < heap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swap_nodes(&heap->array[smallest], &heap->array[idx]);
        min_heapify(heap, smallest);
    }
}

// Извлечение узла с минимальной частотой
HuffNode* extract_min(MinHeap* heap) {
    HuffNode* temp = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    min_heapify(heap, 0);
    return temp;
}

// Вставка нового узла в кучу
StatusCode insert_min_heap(MinHeap* heap, HuffNode* node) {
    if (heap->size == heap->capacity) return ERR_MEMORY_ALLOC;
    heap->size++;
    int i = heap->size - 1;
    while (i && node->freq < heap->array[(i - 1) / 2]->freq) {
        heap->array[i] = heap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    heap->array[i] = node;
    return SUCCESS;
}

// --- Построение дерева и кодов ---

void free_tree(HuffNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

// Рекурсивный обход дерева для генерации кодов из 0 и 1[cite: 21]
void generate_codes(HuffNode* root, char* arr, int top, HuffCode codes[256]) {
    if (root->left) {
        arr[top] = '0';
        generate_codes(root->left, arr, top + 1, codes);
    }
    if (root->right) {
        arr[top] = '1';
        generate_codes(root->right, arr, top + 1, codes);
    }
    // Если это лист (нет потомков)[cite: 20]
    if (!(root->left) && !(root->right)) {
        arr[top] = '\0';
        // Если в файле только один уникальный символ, даем ему код "0"[cite: 21]
        if (top == 0) {
            strcpy(codes[root->byte_val].code, "0");
        } else {
            strcpy(codes[root->byte_val].code, arr);
        }
    }
}

// Главная вычислительная функция, не содержащая принтов[cite: 19]
StatusCode process_file(const char* filepath, HuffCode codes[256], CompressStats* stats) {
    FILE* file = fopen(filepath, "rb"); // Бинарный режим чтения[cite: 20]
    if (!file) return ERR_FILE_OPEN;

    unsigned int freq[256] = {0};
    int byte;
    stats->original_bytes = 0;

    // Считаем частоту каждого байта[cite: 20]
    while ((byte = fgetc(file)) != EOF) {
        freq[byte]++;
        stats->original_bytes++;
    }
    fclose(file);

    if (stats->original_bytes == 0) {
        stats->encoded_bits = 0;
        stats->encoded_bytes = 0;
        stats->savings_percent = 0.0;
        return ERR_EMPTY_FILE; // Пустой файл[cite: 21]
    }

    MinHeap* heap = create_min_heap(256);
    if (!heap) return ERR_MEMORY_ALLOC;

    // Создаем листья для кучи[cite: 20]
    for (int i = 0; i < 256; i++) {
        codes[i].is_present = 0;
        if (freq[i] > 0) {
            codes[i].freq = freq[i];
            codes[i].is_present = 1;
            HuffNode* node = create_node((unsigned char)i, freq[i]);
            insert_min_heap(heap, node);
        }
    }

    // Строим дерево Хаффмана
    while (heap->size > 1) {
        HuffNode* left = extract_min(heap);
        HuffNode* right = extract_min(heap);

        // Вес узла = суммарная частота потомков[cite: 20]
        HuffNode* top = create_node('$', left->freq + right->freq); 
        top->left = left;
        top->right = right;
        insert_min_heap(heap, top);
    }

    HuffNode* root = extract_min(heap);
    char arr[256];
    generate_codes(root, arr, 0, codes);

    // Подсчет итоговых размеров[cite: 21]
    stats->encoded_bits = 0;
    for (int i = 0; i < 256; i++) {
        if (codes[i].is_present) {
            stats->encoded_bits += codes[i].freq * strlen(codes[i].code);
        }
    }
    
    // Перевод в байты с округлением вверх[cite: 21]
    stats->encoded_bytes = (stats->encoded_bits + 7) / 8;
    
    // Формула экономии[cite: 21]
    double N = (double)stats->original_bytes;
    double M = (double)stats->encoded_bytes;
    stats->savings_percent = 100.0 * (1.0 - (M / N));

    // Очистка памяти[cite: 21]
    free_tree(root);
    free(heap->array);
    free(heap);

    return SUCCESS;
}

// --- Секция вывода (Начальник) ---

int main(int argc, char* argv[]) {
    // Валидация аргументов[cite: 19]
    if (argc != 2) {
        printf("Ошибка: неверное количество аргументов.\nИспользование: %s <путь_к_файлу>\n", argv[0]);
        return 1;
    }

    HuffCode codes[256] = {0};
    CompressStats stats = {0};

    StatusCode status = process_file(argv[1], codes, &stats);

    switch (status) {
        case ERR_FILE_OPEN:
            printf("Ошибка: не удалось открыть файл '%s'.\n", argv[1]);
            return 1;
        case ERR_MEMORY_ALLOC:
            printf("Ошибка: не удалось выделить память.\n");
            return 1;
        case ERR_EMPTY_FILE:
            printf("Файл пуст. Таблица кодов пуста.\n");
            printf("Исходный размер: 0 байт\nСжатый размер: 0 бит (0 байт)\nЭкономия: 0.00%%\n");
            return 0; // Для пустого файла экономия 0[cite: 21]
        case SUCCESS:
            break;
        default:
            printf("Неизвестная ошибка.\n");
            return 1;
    }

    // Вывод таблицы: значение (dec), частота, код[cite: 21]
    printf("Значение | Частота | Код Хаффмана\n");
    printf("------------------------------------\n");
    for (int i = 0; i < 256; i++) {
        if (codes[i].is_present) {
            printf("%8d | %7u | %s\n", i, codes[i].freq, codes[i].code);
        }
    }

    // Вывод итоговой статистики[cite: 21]
    printf("------------------------------------\n");
    printf("Исходный размер: %lu байт\n", stats.original_bytes);
    printf("Размер сжатых данных: %lu бит (%lu байт)\n", stats.encoded_bits, stats.encoded_bytes);
    printf("Экономия сжатия: %.2f%%\n", stats.savings_percent);

    return 0;
}