#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 1. Статус-коды
typedef enum {
    SUCCESS = 0,
    ERR_MEMORY,
    ERR_FILE_OPEN,
    ERR_INVALID_DATA,
    ERR_NOT_FOUND
} StatusCode;

// 2. Структуры данных[cite: 21]
typedef struct {
    int day, month, year;
} Date;

typedef struct {
    char* surname;
    char* name;
    char* patronymic;
    Date dob;
    char gender; // 'M' или 'W'
    double income;
} Liver;

typedef struct LiverNode {
    Liver data;
    struct LiverNode* next;
} LiverNode;

// История для Undo[cite: 21]
typedef struct {
    LiverNode** snapshots;
    int count;
    int capacity;
    int mods_since_last_undo;
} History;

// --- Вспомогательные функции ---
int compare_age(Date d1, Date d2) {
    if (d1.year != d2.year) return d2.year - d1.year;
    if (d1.month != d2.month) return d2.month - d1.month;
    return d2.day - d1.day;
}

Liver copy_liver(const Liver* src) {
    Liver dest;
    dest.surname = strdup(src->surname);
    dest.name = strdup(src->name);
    dest.patronymic = src->patronymic ? strdup(src->patronymic) : strdup("");
    dest.dob = src->dob;
    dest.gender = src->gender;
    dest.income = src->income;
    return dest;
}

void free_liver(Liver* l) {
    free(l->surname);
    free(l->name);
    free(l->patronymic);
}

void free_list(LiverNode* head) {
    while (head) {
        LiverNode* temp = head;
        head = head->next;
        free_liver(&temp->data);
        free(temp);
    }
}

LiverNode* clone_list(LiverNode* head) {
    if (!head) return NULL;
    LiverNode* new_head = (LiverNode*)malloc(sizeof(LiverNode));
    new_head->data = copy_liver(&head->data);
    new_head->next = clone_list(head->next);
    return new_head;
}

// --- История (Undo) ---
StatusCode save_snapshot(History* hist, LiverNode* head) {
    if (hist->count >= hist->capacity) {
        hist->capacity = hist->capacity == 0 ? 10 : hist->capacity * 2;
        LiverNode** temp = (LiverNode**)realloc(hist->snapshots, hist->capacity * sizeof(LiverNode*));
        if (!temp) return ERR_MEMORY;
        hist->snapshots = temp;
    }
    hist->snapshots[hist->count++] = clone_list(head);
    hist->mods_since_last_undo++;
    return SUCCESS;
}

LiverNode* perform_undo(History* hist, LiverNode* current_head) {
    int n = hist->mods_since_last_undo;
    if (n == 0) return current_head;
    
    int to_undo = n / 2; // Отменяем N/2 действий[cite: 21]
    if (to_undo == 0 && n > 0) to_undo = 1; 

    int target_idx = hist->count - to_undo - 1;
    if (target_idx < 0) target_idx = 0;

    free_list(current_head);
    LiverNode* restored = clone_list(hist->snapshots[target_idx]);
    
    hist->count = target_idx + 1; // Обрезаем историю
    hist->mods_since_last_undo = 0; // Сбрасываем счетчик[cite: 21]
    
    return restored;
}

// --- Операции со списком ---
StatusCode insert_sorted(LiverNode** head, Liver data) {
    LiverNode* new_node = (LiverNode*)malloc(sizeof(LiverNode));
    if (!new_node) return ERR_MEMORY;
    new_node->data = data;
    new_node->next = NULL;

    if (!*head || compare_age((*head)->data.dob, data.dob) > 0) {
        new_node->next = *head;
        *head = new_node;
        return SUCCESS;
    }

    LiverNode* curr = *head;
    while (curr->next && compare_age(curr->next->data.dob, data.dob) <= 0) {
        curr = curr->next;
    }
    new_node->next = curr->next;
    curr->next = new_node;
    return SUCCESS;
}

// --- Файловый Ввод/Вывод ---
StatusCode load_from_file(const char* filename, LiverNode** head) {
    FILE* f = fopen(filename, "r");
    if (!f) return ERR_FILE_OPEN;

    char sur[256], name[256], pat[256], gen;
    int d, m, y;
    double inc;

    while (fscanf(f, "%255s %255s %255s %d.%d.%d %c %lf", 
                  sur, name, pat, &d, &m, &y, &gen, &inc) == 8) {
        Liver l = {strdup(sur), strdup(name), strdup(pat), {d, m, y}, gen, inc};
        insert_sorted(head, l);
    }
    fclose(f);
    return SUCCESS;
}

void print_list(LiverNode* head) {
    while (head) {
        printf("%s %s, %02d.%02d.%04d, %c, Доход: %.2f\n", 
               head->data.surname, head->data.name, 
               head->data.dob.day, head->data.dob.month, head->data.dob.year, 
               head->data.gender, head->data.income);
        head = head->next;
    }
}

// --- Главное меню ---
int main(int argc, char* argv[]) {
    LiverNode* head = NULL;
    History hist = {0};

    // Загружаем начальные данные
    if (load_from_file("livers.txt", &head) == SUCCESS) {
        save_snapshot(&hist, head);
        hist.mods_since_last_undo = 0; // Исходное состояние не считается модификацией
    }

    int choice;
    while (1) {
        printf("\n1. Показать жителей\n2. Добавить жителя\n3. Undo (Отменить N/2 изменений)\n4. Выход\n> ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            print_list(head);
        } else if (choice == 2) {
            save_snapshot(&hist, head); // Делаем снимок ПЕРЕД изменением
            
            Liver l = {strdup("Ivanov"), strdup("Ivan"), strdup("Ivanovich"), 
                       {1, 1, 2000}, 'M', 50000.0};
            insert_sorted(&head, l);
            printf("Житель добавлен.\n");
        } else if (choice == 3) {
            head = perform_undo(&hist, head);
            printf("Выполнен откат состояния (Undo).\n");
        } else if (choice == 4) {
            break;
        }
    }

    free_list(head);
    for (int i = 0; i < hist.count; i++) free_list(hist.snapshots[i]);
    free(hist.snapshots);
    return 0;
}