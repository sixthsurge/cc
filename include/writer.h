#pragma once 

#include <stdio.h>
#include "common.h"
#include "slice.h"
#include "vec.h"

// Abstraction for writer to file or string buffer

typedef enum WriterKind {
    WriterKindFile,
    WriterKindCharVec,
} WriterKind;

typedef struct Writer {
    WriterKind kind;

    union {
        struct {
            FILE *file;
        } file;

        struct {
            CharVec *buffer;
        } charvec;
    };
} Writer;

Writer file_writer(FILE *file);
Writer charvec_writer(CharVec *buffer);

void writer_write(Writer const *self, char const *string);
void writer_writef(Writer const *self, char const *format_string, ...) ATTRIBUTE_PRINTF_LIKE(2, 3);
void writer_write_charslice(Writer const *self, CharSlice slice);

