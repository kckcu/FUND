#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

// Структура для представления вектора
typedef struct {
    double* coords; 
    int id;         
} Vector;

// Указатель на функцию вычисления нормы. 
// Принимает массив координат и размерность.
typedef double (*NormFunction)(const double* coords, int n);

// Структура для возврата результата по одной норме
typedef struct {
    Vector* longest_vectors; // Массив самых длинных векторов
    int count;               // Количество таких векторов
    double max_norm_val;     // Значение вычисленной нормы
} NormResult;

// 1. Бесконечная норма (норма Чебышева): ||x||_inf = max |x_j|
double norm_inf(const double* coords, int n) {
    double max = 0.0;
    for (int i = 0; i < n; ++i) {
        double val = fabs(coords[i]);
        if (val > max) max = val;
    }
    return max;
}

// 2. p-норма: ||x||_p = ( sum |x_j|^p )^(1/p). 
// Для демонстрации возьмем p = 2 (Евклидова норма)
double norm_p2(const double* coords, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += pow(fabs(coords[i]), 2.0);
    }
    return sqrt(sum);
}

// 3. A-норма: ||x||_A = sqrt( (Ax, x) )
// Для демонстрации используем диагональную положительно определенную матрицу A = diag(1, 2, ..., n)
double norm_matrix_A(const double* coords, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        double A_ii = (double)(i + 1); // Элементы диагонали
        sum += A_ii * coords[i] * coords[i];
    }
    return sqrt(sum);
}

// Функция с переменным числом аргументов
// Аргументы: 
// n - размерность
// num_vectors - количество переданных векторов
// num_norms - количество переданных функций-норм
// ... - экземпляры Vector (num_vectors штук), затем функции NormFunction (num_norms штук)
NormResult* find_longest_vectors(int n, int num_vectors, int num_norms, ...) {
    va_list args;
    va_start(args, num_norms);

    // 1. Считываем векторы
    Vector* vectors = (Vector*)malloc(num_vectors * sizeof(Vector));
    for (int i = 0; i < num_vectors; ++i) {
        vectors[i] = va_arg(args, Vector);
    }

    // Выделяем память под массив результатов (по одному для каждой нормы)
    NormResult* results = (NormResult*)malloc(num_norms * sizeof(NormResult));

    // 2. Считываем функции норм и сразу ищем максимальные векторы
    for (int i = 0; i < num_norms; ++i) {
        NormFunction norm_func = va_arg(args, NormFunction);

        double max_norm = -1.0;
        int max_count = 0;

        // Вычисляем норму для каждого вектора
        double* computed_norms = (double*)malloc(num_vectors * sizeof(double));
        for (int j = 0; j < num_vectors; ++j) {
            computed_norms[j] = norm_func(vectors[j].coords, n);

            // Сравниваем с учетом погрешности вычислений с плавающей точкой
            if (computed_norms[j] > max_norm + 1e-9) {
                max_norm = computed_norms[j];
                max_count = 1;
            } else if (fabs(computed_norms[j] - max_norm) <= 1e-9) {
                max_count++;
            }
        }

        // 3. Формируем результат для текущей нормы
        results[i].max_norm_val = max_norm;
        results[i].count = max_count;
        results[i].longest_vectors = (Vector*)malloc(max_count * sizeof(Vector));

        int idx = 0;
        for (int j = 0; j < num_vectors; ++j) {
            // Собираем все векторы, чья норма равна максимальной
            if (fabs(computed_norms[j] - max_norm) <= 1e-9) {
                results[i].longest_vectors[idx++] = vectors[j];
            }
        }

        free(computed_norms);
    }

    va_end(args);
    free(vectors); // Освобождаем временный массив, сами векторы копируются по значению структуры
    return results;
}

// Демонстрация работы
int main() {
    int n = 3; // Размерность пространства

    // Задаем координаты
    double c1[] = {1.0, 2.0, 3.0};
    double c2[] = {3.0, 2.0, 1.0}; 
    double c3[] = {0.0, 0.0, 4.0}; 
    double c4[] = {-4.0, 0.0, 0.0}; // Имеет такую же inf-норму, как и c3

    Vector v1 = {c1, 1};
    Vector v2 = {c2, 2};
    Vector v3 = {c3, 3};
    Vector v4 = {c4, 4};

    int num_vectors = 4;
    int num_norms = 3;

    // Вызываем нашу variadic функцию
    NormResult* results = find_longest_vectors(
        n, num_vectors, num_norms,
        v1, v2, v3, v4,                       // Передача экземпляров структур
        norm_inf, norm_p2, norm_matrix_A      // Передача указателей на функции
    );

    // Вывод результатов
    const char* norm_names[] = {"Бесконечная норма (Чебышева)", "p-норма (p=2)", "A-норма"};
    
    printf("=== Результаты поиска самых длинных векторов ===\n");
    for (int i = 0; i < num_norms; ++i) {
        printf("\n[%s]\n", norm_names[i]);
        printf("Максимальное значение нормы: %.4f\n", results[i].max_norm_val);
        printf("Найдено векторов: %d\n", results[i].count);
        
        for (int j = 0; j < results[i].count; ++j) {
            printf("  -> Вектор ID %d: [", results[i].longest_vectors[j].id);
            for (int k = 0; k < n; ++k) {
                printf("%.2f%s", results[i].longest_vectors[j].coords[k], k == n - 1 ? "" : ", ");
            }
            printf("]\n");
        }
        
        // Очистка памяти результата
        free(results[i].longest_vectors);
    }
    free(results);

    return 0;
}