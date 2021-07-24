#include "shared.h"

char *read_file_into_buffer(const char *path) {
	FILE *fp = fopen(path, "rb");
	if (!fp)
		error_and_exit(-1, "Can't read file");
	fseek(fp, 0, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *buffer = calloc(length + 1, sizeof(char));
	if (!buffer)
		error_and_exit(-1, "Can't allocate file buffer");
	fread(buffer, sizeof(char), length, fp);
	buffer[length] = 0;
	fclose(fp);
	return buffer;
}

int write_buffer_into_file(void *buffer, size_t bufsize, const char *path) {
	FILE *fp = fopen(path, "wb");
	if (!fp)
		error_and_exit(-1, "Can't read file");
	size_t r = fwrite(buffer, bufsize, 1, fp);
	fclose(fp);
	if (r != 1) {
		printf("r: %zu\n", r);
		return 1;
	}
	return 0;
}
