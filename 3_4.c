#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

// ==========================================
// 1. ОПРЕДЕЛЕНИЕ СТАТУС-КОДОВ
// ==========================================
typedef enum {
    SUCCESS = 0,
    ERR_MEMORY,
    ERR_INVALID_ARG,
    ERR_NOT_FOUND,
    ERR_VALIDATION
} StatusCode;

// ==========================================
// 2. РЕАЛИЗАЦИЯ ТИПА STRING (Пункт 1)
// ==========================================
typedef struct {
    char* data;
    int length;
} String;

// Создание строки
StatusCode string_create(String* str, const char* chars) {
    if (!str || !chars) return ERR_INVALID_ARG;
    str->length = strlen(chars);
    str->data = (char*)malloc((str->length + 1) * sizeof(char));
    if (!str->data) return ERR_MEMORY;
    strcpy(str->data, chars);
    return SUCCESS;
}

// Очистка строки
void string_destroy(String* str) {
    if (str && str->data) {
        free(str->data);
        str->data = NULL;
        str->length = 0;
    }
}

// Отношение порядка (Сначала по длине, потом по алфавиту)
int string_cmp(const String* s1, const String* s2) {
    if (s1->length != s2->length) {
        return s1->length - s2->length;
    }
    return strcmp(s1->data, s2->data);
}

// Эквивалентность (Лексикографически)
bool string_equiv(const String* s1, const String* s2) {
    return strcmp(s1->data, s2->data) == 0;
}

// Копирование в существующий экземпляр
StatusCode string_copy(String* dest, const String* src) {
    if (!dest || !src) return ERR_INVALID_ARG;
    string_destroy(dest); // Чистим старые данные
    return string_create(dest, src->data);
}

// Конкатенация (Приклеивание s2 к s1)
StatusCode string_concat(String* dest, const String* src) {
    if (!dest || !src) return ERR_INVALID_ARG;
    int new_len = dest->length + src->length;
    char* new_data = (char*)realloc(dest->data, (new_len + 1) * sizeof(char));
    if (!new_data) return ERR_MEMORY;
    strcat(new_data, src->data);
    dest->data = new_data;
    dest->length = new_len;
    return SUCCESS;
}

// ==========================================
// 3. СТРУКТУРЫ ПОЧТЫ (Пункт 2)
// ==========================================
typedef struct {
    String city;
    String street;
    int house;
    String building; // Корпус
    int apt;         // Квартира
    String index;    // 6 цифр
} Address;

typedef struct {
    Address receiver;
    double weight;
    String id;            // 14 цифр
    String creation_time; // dd:MM:yyyy hh:mm:ss
    String delivery_time;
} Mail;

typedef struct {
    Address post_office;
    Mail* mails;
    int count;
    int capacity;
} Post;

// --- Вспомогательные функции очистки ---
void address_destroy(Address* addr) {
    string_destroy(&addr->city);
    string_destroy(&addr->street);
    string_destroy(&addr->building);
    string_destroy(&addr->index);
}

void mail_destroy(Mail* mail) {
    address_destroy(&mail->receiver);
    string_destroy(&mail->id);
    string_destroy(&mail->creation_time);
    string_destroy(&mail->delivery_time);
}

void post_destroy(Post* post) {
    address_destroy(&post->post_office);
    for (int i = 0; i < post->count; i++) {
        mail_destroy(&post->mails[i]);
    }
    free(post->mails);
}

// --- Компаратор для сортировки посылок ---
// Первично: индекс получателя, Вторично: ID посылки
int mail_comparator(const void* a, const void* b) {
    const Mail* m1 = (const Mail*)a;
    const Mail* m2 = (const Mail*)b;
    
    int index_cmp = strcmp(m1->receiver.index.data, m2->receiver.index.data);
    if (index_cmp != 0) return index_cmp;
    
    return strcmp(m1->id.data, m2->id.data);
}

// --- Компаратор времени (для доставленных) ---
// Превращаем строку dd:MM:yyyy hh:mm:ss в time_t для сравнения
time_t parse_time(const char* time_str) {
    struct tm t = {0};
    sscanf(time_str, "%d:%d:%d %d:%d:%d", 
           &t.tm_mday, &t.tm_mon, &t.tm_year, 
           &t.tm_hour, &t.tm_min, &t.tm_sec);
    t.tm_mon -= 1;
    t.tm_year -= 1900;
    return mktime(&t);
}

int time_comparator(const void* a, const void* b) {
    const Mail* m1 = (const Mail*)a;
    const Mail* m2 = (const Mail*)b;
    time_t t1 = parse_time(m1->creation_time.data);
    time_t t2 = parse_time(m2->creation_time.data);
    return (t1 > t2) - (t1 < t2);
}

// --- Функции Почтового Отделения (Бизнес-логика) ---
StatusCode post_add_mail(Post* post, Mail new_mail) {
    if (post->count >= post->capacity) {
        post->capacity = post->capacity == 0 ? 2 : post->capacity * 2;
        Mail* new_arr = (Mail*)realloc(post->mails, post->capacity * sizeof(Mail));
        if (!new_arr) return ERR_MEMORY;
        post->mails = new_arr;
    }
    post->mails[post->count++] = new_mail;
    
    // Поддерживаем массив в отсортированном виде всегда
    qsort(post->mails, post->count, sizeof(Mail), mail_comparator);
    return SUCCESS;
}

StatusCode post_find_mail(Post* post, const char* id, Mail** found) {
    for (int i = 0; i < post->count; i++) {
        if (strcmp(post->mails[i].id.data, id) == 0) {
            *found = &post->mails[i];
            return SUCCESS;
        }
    }
    return ERR_NOT_FOUND;
}

// ==========================================
// 4. ИНТЕРАКТИВНЫЙ ДИАЛОГ (Начальник)
// ==========================================
int main() {
    Post my_post = {0};
    
    // Инициализируем адрес текущего отделения
    string_create(&my_post.post_office.city, "Москва");
    string_create(&my_post.post_office.street, "Ленинградское шоссе");
    my_post.post_office.house = 1;
    string_create(&my_post.post_office.building, "A");
    my_post.post_office.apt = 0;
    string_create(&my_post.post_office.index, "125993");

    int choice;
    char buffer[256];

    printf("=== СИСТЕМА УПРАВЛЕНИЯ ПОЧТОЙ ===\n");
    
    while (1) {
        printf("\n1. Добавить отправление\n2. Найти отправление\n3. Показать доставленные\n4. Выход\n> ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            Mail m = {0};
            printf("Введите город: "); scanf("%255s", buffer); string_create(&m.receiver.city, buffer);
            printf("Введите улицу: "); scanf("%255s", buffer); string_create(&m.receiver.street, buffer);
            printf("Дом, квартира: "); scanf("%d %d", &m.receiver.house, &m.receiver.apt);
            string_create(&m.receiver.building, "-"); // Пропускаем для краткости
            
            printf("Индекс (6 цифр): "); scanf("%255s", buffer); string_create(&m.receiver.index, buffer);
            printf("Трек-номер (14 цифр): "); scanf("%255s", buffer); string_create(&m.id, buffer);
            
            m.weight = 1.5;
            string_create(&m.creation_time, "20:10:2026 12:00:00");
            string_create(&m.delivery_time, "25:10:2026 12:00:00");

            if (post_add_mail(&my_post, m) == SUCCESS) {
                printf("[УСПЕХ] Посылка %s добавлена. Массив отсортирован.\n", m.id.data);
            }
        } 
        else if (choice == 2) {
            printf("Введите трек-номер для поиска: ");
            scanf("%255s", buffer);
            Mail* found = NULL;
            if (post_find_mail(&my_post, buffer, &found) == SUCCESS) {
                printf("Найдена посылка: Индекс %s, Вес %.2f, Создана: %s\n", 
                       found->receiver.index.data, found->weight, found->creation_time.data);
            } else {
                printf("[ОШИБКА] Посылка не найдена.\n");
            }
        }
        else if (choice == 3) {
            printf("--- Доставленные отправления ---\n");
            // Копируем массив, чтобы отсортировать его по времени создания
            Mail* temp = malloc(my_post.count * sizeof(Mail));
            memcpy(temp, my_post.mails, my_post.count * sizeof(Mail));
            qsort(temp, my_post.count, sizeof(Mail), time_comparator);

            for(int i = 0; i < my_post.count; i++) {
                printf("Трек: %s | Индекс: %s | Создано: %s\n", 
                       temp[i].id.data, temp[i].receiver.index.data, temp[i].creation_time.data);
            }
            free(temp);
        }
        else if (choice == 4) {
            break;
        }
    }

    post_destroy(&my_post);
    printf("Система завершила работу. Память очищена.\n");
    return 0;
}