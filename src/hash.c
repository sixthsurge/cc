#include "hash.h"

usize charslice_hash_djb2(struct CharSlice const s) {
    usize hash = 5381;

    for (usize i = 0; i < s.len; ++i) {
        char const c = s.ptr[i];
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}
