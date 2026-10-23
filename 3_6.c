#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// ==========================================
// 1. СТАТУС-КОДЫ И СТРУКТУРЫ
// ==========================================
typedef enum {
    SUCCESS = 0,
    ERR_MEMORY,
    ERR_FILE_OPEN,
    ERR_INVALID_DATA
} StatusCode;

typedef enum {
    START = 0,
    INTERMEDIATE,
    END
} StopType;

// Узел 3-го уровня: Конкретная остановка автобуса
typedef struct StopNode {
    time_t arr_time;
    time_t dep_time;
    StopType type;
    double x, y; // Координаты остановки
    struct StopNode* next;
} StopNode;

// Узел 2-го уровня: Маршрут (от START до END)
typedef struct RouteNode {
    StopNode* stops_head;
    struct RouteNode* next;
} RouteNode;

// Узел 1-го уровня: Транспортное средство (Автобус)
typedef struct VehicleNode {
    char* vehicle_id;
    RouteNode* routes_head;
    struct VehicleNode* next;
} VehicleNode;

// ==========================================
// 2. ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ (Парсинг времени)
// ==========================================
time_t parse_time(const char* str) {
    struct tm t = {0};
    // Ожидаемый формат: dd.MM.yyyy hh:mm:ss[cite: 14]
    sscanf(str, "%d.%d.%d %d:%d:%d", 
           &t.tm_mday, &t.tm_mon, &t.tm_year, 
           &t.tm_hour, &t.tm_min, &t.tm_sec);
    t.tm_mon -= 1;
    t.tm_year -= 1900;
    return mktime(&t);
}

// ==========================================
// 3. РАБОТА СО СПИСКАМИ (Бизнес-логика)
// ==========================================

// Поиск или создание автобуса в главном списке
VehicleNode* get_or_create_vehicle(VehicleNode** head, const char* v_id) {
    VehicleNode* curr = *head;
    while (curr) {
        if (strcmp(curr->vehicle_id, v_id) == 0) return curr;
        curr = curr->next;
    }
    // Если не нашли, создаем новый
    VehicleNode* new_v = (VehicleNode*)calloc(1, sizeof(VehicleNode));
    new_v->vehicle_id = strdup(v_id);
    new_v->next = *head;
    *head = new_v;
    return new_v;
}

// Добавление остановки в хронологическом порядке
void insert_stop_sorted(StopNode** head, StopNode* new_stop) {
    if (!*head || (*head)->arr_time > new_stop->arr_time) {
        new_stop->next = *head;
        *head = new_stop;
        return;
    }
    StopNode* curr = *head;
    while (curr->next && curr->next->arr_time <= new_stop->arr_time) {
        curr = curr->next;
    }
    new_stop->next = curr->next;
    curr->next = new_stop;
}

// Группировка отсортированных остановок в маршруты (от START до END)
void form_routes(VehicleNode* v) {
    // Упрощенная логика: все остановки пока лежат в одном временном маршруте.
    // По заданию нужно разбивать их на куски от START до END[cite: 14].
    if (!v->routes_head || !v->routes_head->stops_head) return;
    
    StopNode* curr = v->routes_head->stops_head;
    v->routes_head->stops_head = NULL; // Очищаем временный корень
    
    RouteNode* current_route = NULL;
    
    while (curr) {
        StopNode* next_node = curr->next;
        curr->next = NULL;
        
        if (curr->type == START || current_route == NULL) {
            RouteNode* new_r = (RouteNode*)calloc(1, sizeof(RouteNode));
            new_r->next = v->routes_head;
            v->routes_head = new_r;
            current_route = new_r;
        }
        
        // Добавляем в конец текущего маршрута
        if (!current_route->stops_head) {
            current_route->stops_head = curr;
        } else {
            StopNode* tail = current_route->stops_head;
            while(tail->next) tail = tail->next;
            tail->next = curr;
        }
        
        if (curr->type == END) current_route = NULL; // Маршрут завершен
        
        curr = next_node;
    }
}

// ==========================================
// 4. ЧТЕНИЕ ФАЙЛОВ
// ==========================================
StatusCode process_file(const char* filename, VehicleNode** main_list) {
    FILE* f = fopen(filename, "r");
    if (!f) return ERR_FILE_OPEN;

    double x, y;
    // Предполагаем, что первая строка файла - координаты остановки
    if (fscanf(f, "%lf %lf\n", &x, &y) != 2) {
        fclose(f);
        return ERR_INVALID_DATA;
    }

    char v_id[256], t_arr[512], t_dep[512], marker[256];
    char arr_d[256], arr_t[256], dep_d[256], dep_t[256];

    // Читаем: Номер | Дата_приб | Время_приб | Дата_отпр | Время_отпр | Маркер
    while (fscanf(f, "%255s %255s %255s %255s %255s %255s", 
           v_id, arr_d, arr_t, dep_d, dep_t, marker) == 6) {
        
        VehicleNode* v = get_or_create_vehicle(main_list, v_id);
        
        // Если у автобуса еще нет маршрутов, создаем временный контейнер
        if (!v->routes_head) {
            v->routes_head = (RouteNode*)calloc(1, sizeof(RouteNode));
        }

        StopNode* stop = (StopNode*)calloc(1, sizeof(StopNode));
        stop->x = x; stop->y = y;
        
        sprintf(t_arr, "%s %s", arr_d, arr_t);
        sprintf(t_dep, "%s %s", dep_d, dep_t);
        stop->arr_time = parse_time(t_arr);
        stop->dep_time = parse_time(t_dep);
        
        if (strcmp(marker, "START") == 0) stop->type = START;
        else if (strcmp(marker, "END") == 0) stop->type = END;
        else stop->type = INTERMEDIATE;

        // Вставляем с сортировкой по времени[cite: 14]
        insert_stop_sorted(&(v->routes_head->stops_head), stop);
    }

    fclose(f);
    return SUCCESS;
}

// ==========================================
// 5. ИНТЕРАКТИВНОЕ МЕНЮ И СТАТИСТИКА (Начальник)
// ==========================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Использование: %s <файл_остановки1> <файл_остановки2> ...\n", argv[0]);
        return 1;
    }

    VehicleNode* main_list = NULL;

    // Читаем все файлы[cite: 14]
    for (int i = 1; i < argc; i++) {
        if (process_file(argv[i], &main_list) != SUCCESS) {
            printf("Ошибка чтения файла: %s\n", argv[i]);
        }
    }

    // Формируем маршруты
    VehicleNode* curr_v = main_list;
    while (curr_v) {
        form_routes(curr_v);
        curr_v = curr_v->next;
    }

    printf("=== СИСТЕМА УЧЕТА ТРАНСПОРТА ===\n");
    int choice;
    while (1) {
        printf("\n1. Найти ТС с наибольшим числом маршрутов\n2. Найти ТС с самым долгим простоем\n3. Выход\n> ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            int max_routes = -1;
            char* best_v = NULL;
            curr_v = main_list;
            
            while (curr_v) {
                int r_count = 0;
                RouteNode* r = curr_v->routes_head;
                while(r) { r_count++; r = r->next; }
                
                if (r_count > max_routes) { max_routes = r_count; best_v = curr_v->vehicle_id; }
                curr_v = curr_v->next;
            }
            if (best_v) printf("Лидер по числу маршрутов: %s (%d шт.)\n", best_v, max_routes);
        }
        else if (choice == 2) {
            double max_idle = -1;
            char* best_v = NULL;
            curr_v = main_list;
            
            while (curr_v) {
                double idle_sum = 0;
                RouteNode* r = curr_v->routes_head;
                while(r) {
                    StopNode* s = r->stops_head;
                    while(s) {
                        idle_sum += difftime(s->dep_time, s->arr_time);
                        s = s->next;
                    }
                    r = r->next;
                }
                if (idle_sum > max_idle) { max_idle = idle_sum; best_v = curr_v->vehicle_id; }
                curr_v = curr_v->next;
            }
            if (best_v) printf("Самый долгий простой у: %s (%.0f секунд)\n", best_v, max_idle);
        }
        else if (choice == 3) {
            break;
        }
    }
    printf("Работа завершена.\n");
    return 0;
}