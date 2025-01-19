#pragma once

#include "cc/slice.h"

// Source:
// http://www.cse.yorku.ca/~oz/hash.html
usize charslice_hash_djb2(struct CharSlice s);
