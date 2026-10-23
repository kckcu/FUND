#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FIXED_SIZE 15

void swap_min_max(int *arr, int size) {
    if (size <= 1) return;
    
    int min_idx = 0;
    int max_idx = 0;
    
    for (int i = 1; i < size; i++) {
        if (arr[i] < arr[min_idx]) {
            min_idx = i;
        }
        if (arr[i] > arr[max_idx]) {
            max_idx = i;
        }
    }
    
    int temp = arr[min_idx];
    arr[min_idx] = arr[max_idx];
    arr[max_idx] = temp;
}

int find_closest(int target, int *B, int size_b) {
    int closest = B[0];
    int min_diff = abs(target - closest);
    
    for (int i = 1; i < size_b; i++) {
        int diff = abs(target - B[i]);
        if (diff < min_diff) {
            min_diff = diff;
            closest = B[i];
        }
    }
    return closest;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <a> <b>\n", argv[0]);
        return 1;
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    
    if (a > b) {
        int temp = a;
        a = b;
        b = temp;
    }

    srand(time(NULL));

    printf("[--- ЗАДАЧА 1 ---]\n");
    int arr[FIXED_SIZE];
    
    printf("Исходный массив (диапазон [%d..%d]):\n", a, b);
    for (int i = 0; i < FIXED_SIZE; i++) {
        arr[i] = rand() % (b - a + 1) + a; 
        printf("%d ", arr[i]);
    }
    printf("\n");

    swap_min_max(arr, FIXED_SIZE);

    printf("Массив после обмена мин и макс:\n");
    for (int i = 0; i < FIXED_SIZE; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n\n");

    printf("[--- ЗАДАЧА 2 ---]\n");
    int size_a = rand() % (10000 - 10 + 1) + 10;
    int size_b = rand() % (10000 - 10 + 1) + 10;
    
    printf("Сгенерированы размеры: A = %d, B = %d\n", size_a, size_b);

    int *A = (int*)malloc(size_a * sizeof(int));
    int *B = (int*)malloc(size_b * sizeof(int));
    int *C = (int*)malloc(size_a * sizeof(int));

    if (!A || !B || !C) {
        fprintf(stderr, "Ошибка выделения памяти!\n");
        return 1;
    }

    for (int i = 0; i < size_a; i++) {
        A[i] = rand() % (1000 - (-1000) + 1) + (-1000);
    }
    for (int i = 0; i < size_b; i++) {
        B[i] = rand() % (1000 - (-1000) + 1) + (-1000);
    }

    for (int i = 0; i < size_a; i++) {
        int closest_val = find_closest(A[i], B, size_b);
        C[i] = A[i] + closest_val;
    }

    int print_limit = (size_a < 10) ? size_a : 10;
    printf("Демонстрация первых %d элементов массива C:\n", print_limit);
    for (int i = 0; i < print_limit; i++) {
        printf("A[%d] = %5d | Ближайший в B = %5d | C[%d] = %5d\n", 
               i, A[i], find_closest(A[i], B, size_b), i, C[i]);
    }

    free(A);
    free(B);
    free(C);

    return 0;
}