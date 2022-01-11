#ifndef IO_H
#define IO_H

#include <stdlib.h>

char *io_file_read(const char *path);
int io_file_write(void *buffer, size_t size, const char *path);

#endif
