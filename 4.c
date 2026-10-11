#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>


//глубокая проверка, что оба файла разные
bool is_same_file(const chaar* path1, const char* path2) {
    struct stat stat1, stat2;

    if (stat(path1, &stat1) == 0 && stat(path2, &stat2) == 0) {
        if (stat1.st_dev == stat2.st_dev && stat1.st_ino == stat2.st_ino) {
            return true;
        }
    }
    return false;
}


char* generate_output_filename(const char* filepath) {
    const char* slash1 = strrchr(filepath, '/');
    const char* slash2 = strrchr(filepath, '\\');
    const char* last_slash = (slash1 > slash2) ? slash1 : slash2;

    char* out_path = (char*)malloc(strlen(filepath) + 5);
    if (!out_path) {
        fprintf(stderr, "error: failed to allocate memory\n");
        exit(1);
    }

    if (last_slash) {
        int dir_len = last_slash - filepath + 1;
        srtncpy(out_path, filepath, dir_len);
        out_path[dir_len] = '\0';
        strcat(out_path, "out_");
        strcat(out_path, last_slash + 1);
    } else {
        strcpy(out_path, "out_");
        strcat(out_path, filepath);
    }
    return out_path;
} 

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <flag> <input_file> [output_file]\n", argv[0]);
        return 1;
    }

    char *flag = argv[1];
    if (flag[0] != '-' && flag[0] != '/') {
        fprintf(stderr, "error: flag must start with '-' or '/'.\n");
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
        fprintf(stderr, "error: output file path is required when using a flag with 'n'.\n");
        return 1;
    }
    if (!has_n && argc != 3) {
        fprintf(stderr, "error: too many arguments for a flag without 'n'.\n");
        return 1;
    }

    if (action != 'd' && action != 'i' && action != 's' && action != 'a') {
        fprintf(stderr, "error: unknown action '%c'. Allowed: d, i, s, a.\n", action);
        return 1;
    }

    char *input_filepath = argv[2];
    char *output_filepath = has_n ? argv[3] : generate_output_filename(input_filepath);

    // ВЫЗОВ НАШЕЙ ГЛУБОКОЙ ПРОВЕРКИ
    if (is_same_file(input_filepath, output_filepath)) {
        fprintf(stderr, "error: input and output files point to the same physical file on disk. This will destroy data.\n");
        if (!has_n) free(output_filepath);
        return 1;
    }

    FILE* in_file = fopen(input_filepath, "r");
    if (!in_file) {
        fprintf(stderr, "error: failed to open input file %s\n", input_filepath);
        if (!has_n) free(output_filepath);
        return 1;
    }

    FILE* out_file = fopen(output_filepath, "w");
    if (!out_file) {
        fprintf(stderr, "error: failed to create output file %s\n", output_filepath);
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

    printf("Processing completed. Result saved to: %s\n", output_filepath);

    fclose(in_file);
    fclose(out_file);
    if (!has_n) {
        free(output_filepath);
    }

    return 0;
}