#include "cc/writer.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cc/slice.h"
#include "cc/vec.h"

struct Writer file_writer(FILE *const file) {
    return (struct Writer) {
        .kind = WriterKindFile,
        .variant.file = { .file = file },
    };
}

struct Writer charvec_writer(struct CharVec *const buffer) {
    return (struct Writer) {
        .kind = WriterKindCharVec,
        .variant.charvec = { .buffer = buffer },
    };
}

void writer_write(
    struct Writer const *const self,
    char const *const string
) {
    switch (self->kind) {
        case WriterKindFile: {
            fputs(string, self->variant.file.file);
            break;
        }

        case WriterKindCharVec: {
            charvec_push_slice(
                self->variant.charvec.buffer, 
                charslice_from_cstr(string)
            );
            break;
        }
    }
}

void writer_writef(
    struct Writer const *const self,
    char const *const format_string, 
    ...
) {
    va_list args;
    va_start(args, format_string);

    switch (self->kind) {
        case WriterKindFile: {
            vfprintf(self->variant.file.file, format_string, args);
            break;
        }

        case WriterKindCharVec: {
            char *buf;
            vasprintf(&buf, format_string, args);
            charvec_push_slice(
                self->variant.charvec.buffer, 
                charslice_from_cstr(buf)
            );
            free(buf);
            break;
        }
    }

    va_end(args);
}

void writer_write_charslice(struct Writer const *self, struct CharSlice slice) {
    switch (self->kind) {
        case WriterKindFile: {
            fputs(charslice_as_cstr(slice), self->variant.file.file);
            break;
        }
        case WriterKindCharVec: {
            charvec_push_slice(self->variant.charvec.buffer, slice);
            break;
        }
    }
}
