#include <stdio.h>
#include "io.h"

char *io_file_read(const char *path) {
    FILE *fp = fopen(path, "r");

    if (!fp) {
        printf("Cannot read file %s\n", path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    int length = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    char *buffer = malloc((length + 1) * sizeof(char));
    if (!buffer) {
        printf("Cannot allocate file buffer for %s\n", path);
        return NULL;
    }

    fread(buffer, sizeof(char), length, fp);
    buffer[length] = 0;

    fclose(fp);

    printf("File loaded. %s\n", path);
    return buffer;
}

int io_file_write(void *buffer, size_t size, const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        printf("Cannot write file %s\n", path);
        return 1;
    }

    size_t result = fwrite(buffer, size, 1, fp);

    fclose(fp);

    if (result != 1) {
        printf("Incorrect chunks written. Expected 1, got %zu.\n", result);
        return 1;
    }

    printf("File created. %s\n", path);
    return 0;
}