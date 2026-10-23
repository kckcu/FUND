#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int compare_doubles(const void *a, const void *b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

bool next_permutation(double *arr, int n) {
    int i = n - 2;
    while (i >= 0 && arr[i] >= arr[i + 1]) {
        i--;
    }
    if (i < 0) return false; 
    
    int j = n - 1;
    while (arr[j] <= arr[i]) {
        j--;
    }
    

    double temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
    
    int left = i + 1;
    int right = n - 1;
    while (left < right) {
        temp = arr[left];
        arr[left] = arr[right];
        arr[right] = temp;
        left++;
        right--;
    }
    return true;
}


bool areEqual(double a, double b, double eps) {
    return fabs(a - b) <= eps; 
}

void solveEquation(double a, double b, double c, double eps) {
    printf("Уравнение %.2lfx^2 + %.2lfx + %.2lf = 0: ", a, b, c);
    
    if (areEqual(a, 0.0, eps)) {
        if (areEqual(b, 0.0, eps)) {
            if (areEqual(c, 0.0, eps)) printf("Бесконечно много решений\n");
            else printf("Нет решений\n");
        } else {
            printf("x = %.4lf\n", -c / b);
        }
        return;
    }
    

    double discriminant = b * b - 4 * a * c;
    
    if (discriminant > eps) {
        double x1 = (-b + sqrt(discriminant)) / (2 * a);
        double x2 = (-b - sqrt(discriminant)) / (2 * a);
        printf("x1 = %.4lf, x2 = %.4lf\n", x1, x2);
    } else if (areEqual(discriminant, 0.0, eps)) {
        printf("x = %.4lf\n", -b / (2 * a));
    } else {
        printf("Нет действительных корней\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Ошибка: Не передан флаг.\n");
        return 1;
    }

    char* flag = argv[1];
    if (flag[0] != '-' && flag[0] != '/') {
        fprintf(stderr, "Ошибка: Флаг должен начинаться с '-' или '/'.\n");
        return 1;
    }

    char flagChar = flag[1];

    if (flagChar == 'q') {
        if (argc != 6) {
            fprintf(stderr, "Ошибка: Для флага -q требуется 4 параметра (eps, a, b, c).\n");
            return 1;
        }
        
        double eps = atof(argv[2]);
        if (eps <= 0) {
            fprintf(stderr, "Ошибка: Точность (эпсилон) должна быть больше нуля.\n");
            return 1;
        }

        double coeffs[3] = {atof(argv[3]), atof(argv[4]), atof(argv[5])};
        
   
        qsort(coeffs, 3, sizeof(double), compare_doubles);
        
        printf("Решения для всех уникальных перестановок:\n");
        do {
            solveEquation(coeffs[0], coeffs[1], coeffs[2], eps);
        } while (next_permutation(coeffs, 3));

    } else if (flagChar == 'm') {
        if (argc != 4) {
            fprintf(stderr, "Ошибка: Для флага -m требуется 2 параметра (целые числа).\n");
            return 1;
        }
        

        int a = atoi(argv[2]);
        int b = atoi(argv[3]);
        
        if (a == 0 || b == 0) {
            fprintf(stderr, "Ошибка: Числа должны быть ненулевыми.\n");
            return 1;
        }

        if (a % b == 0) {
            printf("Число %d кратно %d\n", a, b);
        } else {
            printf("Число %d НЕ кратно %d\n", a, b);
        }

    } else if (flagChar == 't') {
        if (argc != 6) {
            fprintf(stderr, "Ошибка: Для флага -t требуется 4 параметра (eps, side1, side2, side3).\n");
            return 1;
        }
        
        double eps = atof(argv[2]);
        if (eps <= 0) {
            fprintf(stderr, "Ошибка: Точность (эпсилон) должна быть больше нуля.\n");
            return 1;
        }

        double sides[3] = {atof(argv[3]), atof(argv[4]), atof(argv[5])};
        

        qsort(sides, 3, sizeof(double), compare_doubles);

        if (sides[0] <= eps || sides[0] + sides[1] <= sides[2] + eps) {
            printf("Треугольник с такими сторонами не существует.\n");
            return 0;
        }

        if (areEqual(sides[2] * sides[2], sides[0] * sides[0] + sides[1] * sides[1], eps)) {
            printf("Эти стороны МОГУТ образовывать прямоугольный треугольник.\n");
        } else {
            printf("Эти стороны НЕ МОГУТ образовывать прямоугольный треугольник.\n");
        }

    } else {
        fprintf(stderr, "Ошибка: Неизвестный флаг.\n");
        return 1;
    }

    return 0;
}