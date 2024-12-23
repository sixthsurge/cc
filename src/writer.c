#include "writer.h"

#include "slice.h"
#include "vec.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

Writer file_writer(FILE *const file) {
    return (Writer) {
        .kind = WriterKindFile,
        .file = { .file = file },
    };
}

Writer charvec_writer(CharVec *const buffer) {
    return (Writer) {
        .kind = WriterKindCharVec,
        .charvec = { .buffer = buffer },
    };
}

void writer_write(
    Writer const *const self,
    char const *const string
) {
    switch (self->kind) {
        case WriterKindFile: {
            fputs(string, self->file.file);
            break;
        }

        case WriterKindCharVec: {
            charvec_push_slice(
                self->charvec.buffer, 
                charslice_from_cstr(string)
            );
            break;
        }
    }
}

void writer_writef(
    Writer const *const self,
    char const *const format_string, 
    ...
) {
    va_list args;
    va_start(args, format_string);

    switch (self->kind) {
        case WriterKindFile: {
            vfprintf(self->file.file, format_string, args);
            break;
        }

        case WriterKindCharVec: {
            char *buf;
            vasprintf(&buf, format_string, args);
            charvec_push_slice(
                self->charvec.buffer, 
                charslice_from_cstr(buf)
            );
            free(buf);
            break;
        }
    }

    va_end(args);
}

void writer_write_charslice(Writer const *self, CharSlice slice) {
    switch (self->kind) {
        case WriterKindFile: {
            fputs(charslice_as_cstr(slice), self->file.file);
            break;
        }

        case WriterKindCharVec: {
            charvec_push_slice(self->charvec.buffer, slice);
            break;
        }
    }
}
