#include "read_file_to_string.h"

#include <stdio.h>
#include <stdlib.h>

char *read_file_to_string(char const *const path) {
    // https://stackoverflow.com/a/174552

    FILE *const f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    int const length = ftell(f);

    fseek(f, 0, SEEK_SET);

    char *const buffer = (char *) malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, f);
        // append null terminator
        buffer[length] = '\0';
    }

    fclose(f);
    return buffer;
}
