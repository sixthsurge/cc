#include "cc/slice.h"

#include <stdlib.h>
#include <string.h>

struct CharSlice charslice_from_cstr(char const *cstr) {
    usize const len = strlen(cstr);

    return (struct CharSlice) {
        .ptr = (char *) cstr,
        .len = len,
    };
}

char *charslice_as_cstr(struct CharSlice const self) {
    char *buf = (char *) malloc(self.len + 1u);
    memcpy(buf, self.ptr, self.len);
    buf[self.len] = '\0';
    return buf;
}

bool charslice_eq(struct CharSlice const self, struct CharSlice const other) {
    if (self.len != other.len) {
        return false;
    }

    for (usize i = 0u; i < self.len; i += 1) {
        if (self.ptr[i] != other.ptr[i]) {
            return false;
        }
    }
    
    return true;
}

bool charslice_eq_cstr(struct CharSlice const self, char const *const cstr) {
    return charslice_eq(self, charslice_from_cstr(cstr));
}

