#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

// Главная функция: переразложение многочлена по степеням (x - a)
void re_expand_poly(double eps, double a, double **result_coeffs, int degree, ...) {
    // Защита от дурака: отрицательная степень или пустой указатель
    if (degree < 0 || result_coeffs == NULL) return;

    // Выделяем память под массив новых коэффициентов g. 
    // Размер: степень + 1 (так как есть свободный член g_0)
    *result_coeffs = (double *)malloc((degree + 1) * sizeof(double));
    if (*result_coeffs == NULL) return;

    va_list args;
    va_start(args, degree);

    // 1. Сначала просто копируем исходные коэффициенты f_0 ... f_n в наш массив
    for (int i = 0; i <= degree; i++) {
        (*result_coeffs)[i] = va_arg(args, double);
    }
    va_end(args);

    // 2. Схема Горнера: сдвигаем "точку отсчета" на значение 'a'
    for (int i = 0; i < degree; i++) {
        for (int j = degree - 1; j >= i; j--) {
            (*result_coeffs)[j] = (*result_coeffs)[j] + (*result_coeffs)[j + 1] * a;
        }
    }

    // 3. Зачистка вычислительного мусора с помощью эпсилон
    for (int i = 0; i <= degree; i++) {
        if (fabs((*result_coeffs)[i]) < eps) {
            (*result_coeffs)[i] = 0.0;
        }
    }
}

// Демонстрация работы
int main() {
    double eps = 1e-6; // Наша точность
    double *g_coeffs = NULL; // Тот самый указатель, куда запишется результат

    // ТЕСТ 1:
    // Исходный многочлен: f(x) = 1 + 2x + 3x^2 (степень 2)
    // Сдвигаем на a = 1. То есть ищем разложение по (x - 1)
    // Ожидаемый ответ: 6 + 8(x-1) + 3(x-1)^2
    
    int degree1 = 2;
    double a1 = 1.0;
    
    // Передаем: эпсилон, a, АДРЕС указателя (&g_coeffs), степень, и сами коэффициенты f0, f1, f2
    re_expand_poly(eps, a1, &g_coeffs, degree1, 1.0, 2.0, 3.0);

    printf("=== Сдвиг многочлена f(x) = 1 + 2x + 3x^2 на a = %.1f ===\n", a1);
    if (g_coeffs) {
        for (int i = 0; i <= degree1; i++) {
            printf("g_%d = %.2f\n", i, g_coeffs[i]);
        }
        free(g_coeffs); // Обязательно убираем за собой
    }
    printf("------------------------------------------------------\n");

    // ТЕСТ 2:
    // Исходный многочлен: f(x) = -5 + 0x + 2x^2 + 1x^3 (степень 3)
    // Сдвигаем на a = -2. Ищем разложение по (x + 2)
    int degree2 = 3;
    double a2 = -2.0;
    
    re_expand_poly(eps, a2, &g_coeffs, degree2, -5.0, 0.0, 2.0, 1.0);

    printf("=== Сдвиг многочлена f(x) = -5 + 2x^2 + x^3 на a = %.1f ===\n", a2);
    if (g_coeffs) {
        for (int i = 0; i <= degree2; i++) {
            printf("g_%d = %.2f\n", i, g_coeffs[i]);
        }
        free(g_coeffs);
    }

    return 0;
}