#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

char* generate_output_filename(const char* filepath) {

    const char* slash1 = strrchr(filepath, '/');
    const char* slash2 = strrchr(filepath, '\\');
    const char* last_slash = (slash1 > slash2) ? slash1 : slash2;

    char* out_path = (char*)malloc(strlen(filepath) + 5);
    if (!out_path) {
        fprintf(stderr, "Ошибка: не удалось выделить память.\n");
        exit(1);
    }

    if (last_slash) {
        int dir_len = last_slash - filepath + 1;
        strncpy(out_path, filepath, dir_len);
        out_path[dir_len] = '\0';
        strcat(out_path, "out_");
        strcat(out_path, last_slash + 1);
    } else {
        strcpy(out_path, "out_");
        strcat(out_path, filepath);
    }

    return out_path;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Использование: %s <флаг> <входной_файл> [выходной_файл]\n", argv[0]);
        return 1;
    }

    char* flag = argv[1];
    if (flag[0] != '-' && flag[0] != '/') {
        fprintf(stderr, "Ошибка: флаг должен начинаться с '-' или '/'.\n");
        return 1;
    }

    bool has_n = false;
    char action = '\0';


    if (flag[1] == 'n') {
        has_n = true;
        action = flag[2]; 
    } else {
        action = flag[1]; 
    }

    if (has_n && argc != 4) {
        fprintf(stderr, "Ошибка: при использовании флага с 'n' необходимо указать путь к выходному файлу.\n");
        return 1;
    }
    if (!has_n && argc != 3) {
        fprintf(stderr, "Ошибка: слишком много аргументов для флага без 'n'.\n");
        return 1;
    }

    if (action != 'd' && action != 'i' && action != 's' && action != 'a') {
        fprintf(stderr, "Ошибка: неизвестное действие '%c'. Допустимые: d, i, s, a.\n", action);
        return 1;
    }

    char* input_filepath = argv[2];
    char* output_filepath = has_n ? argv[3] : generate_output_filename(input_filepath);

    FILE* in_file = fopen(input_filepath, "r");
    if (!in_file) {
        fprintf(stderr, "Ошибка: не удалось открыть входной файл %s\n", input_filepath);
        if (!has_n) free(output_filepath);
        return 1;
    }

    FILE* out_file = fopen(output_filepath, "w");
    if (!out_file) {
        fprintf(stderr, "Ошибка: не удалось создать выходной файл %s\n", output_filepath);
        fclose(in_file);
        if (!has_n) free(output_filepath);
        return 1;
    }

    int c;
    int count = 0;
    bool line_started = false;


    while ((c = fgetc(in_file)) != EOF) {
        if (action == 'd') {
            if (!isdigit(c)) {
                fputc(c, out_file);
            }
        } 
        else if (action == 'i') {
            line_started = true;
            if (isalpha(c)) {
                count++;
            }
            if (c == '\n') {
                fprintf(out_file, "%d\n", count);
                count = 0;
                line_started = false;
            }
        } 
        else if (action == 's') {
            line_started = true;
            if (!isalpha(c) && !isdigit(c) && c != ' ' && c != '\n') {
                count++;
            }
            if (c == '\n') {
                fprintf(out_file, "%d\n", count);
                count = 0;
                line_started = false;
            }
        } 
        else if (action == 'a') {
            if (isdigit(c) || c == '\n') {
                fputc(c, out_file);
            } else {
                fprintf(out_file, "%02X", (unsigned char)c);
            }
        }
    }

    if (line_started && (action == 'i' || action == 's')) {
        fprintf(out_file, "%d\n", count);
    }

    printf("Обработка завершена. Результат записан в: %s\n", output_filepath);

    fclose(in_file);
    fclose(out_file);
    if (!has_n) {
        free(output_filepath); 
    }

    return 0;
}