#include <stdio.h>
#include "io.h"

char *io_file_read(const char *path) {
    FILE *fp = fopen(path, "rb");

    if (!fp) {
        printf("Cannot read file %s\n", path);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);

    int length = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    char *buffer = malloc((length + 1) * sizeof(char));
    if (!buffer) {
        printf("Cannot allocate file buffer for %s\n", path);
        exit(1);
    }

    fread(buffer, sizeof(char), length, fp);
    buffer[length] = 0;

    fclose(fp);

    return buffer;
}

int io_file_write(void *buffer, size_t size, const char *path) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        printf("Cannot read file %s\n", path);
        exit(1);
    }

    size_t r = fwrite(buffer, size, 1, fp);
    fclose(fp);

    if (r != 1) {
        printf("Problem writing file %s. Expected writing 1 element but got %zu", path, r);
        return 1;
    }

    return 0;
}