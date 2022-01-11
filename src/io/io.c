#include <stdio.h>
#include "io.h"

char *io_file_read(const char *path) {
    FILE *fp = fopen(path, "r");

    if (!fp) {
        printf("Cannot read file %s\n", path);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);

    ssize_t length = ftell(fp);

    if (length == -1L) {
        printf("Could not assertain length of file %s\n", path);
        return NULL;
    }

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
