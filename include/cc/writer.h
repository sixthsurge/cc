#pragma once 

#include <stdio.h>

#include "cc/common.h"
#include "cc/slice.h"
#include "cc/vec.h"

// Abstraction for writer to file or string buffer

enum WriterKind {
    WriterKindFile,
    WriterKindCharVec,
};

struct Writer {
    enum WriterKind kind;

    union {
        struct {
            FILE *file;
        } file;

        struct {
            struct CharVec *buffer;
        } charvec;
    };
};

struct Writer file_writer(FILE *file);
struct Writer charvec_writer(struct CharVec *buffer);

void writer_write(struct Writer const *self, char const *string);
void writer_writef(struct Writer const *self, char const *format_string, ...) ATTRIBUTE_PRINTF_LIKE(2, 3);
void writer_write_charslice(struct Writer const *self, struct CharSlice slice);

