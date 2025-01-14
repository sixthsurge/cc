// define 
// MAP_TYPE            | Name for map type e.g. StringIntMap
// MAP_KEY_TYPE        | Type of keys e.g. char const *
// MAP_VALUE_TYPE      | Type of values e.g. int
// MAP_FUNCTION_PREFIX | Prefix for functions on the map, e.g. stringintmap_
// MAP_KEY_HASH_FN     | Hash function for keys   (MAP_KEY_TYPE -> usize)
// MAP_KEY_EQ_FN       | Hash function for values (MAP_KEY_TYPE, MAP_KEY_TYPE -> bool)

#include <stdlib.h>
#include <string.h>

#define MAP_CONCAT1(a, b) a##b
#define MAP_CONCAT2(a, b) MAP_CONCAT1(a, b)
#define MAP_PREFIX(function) MAP_CONCAT2(MAP_FUNCTION_PREFIX, function)

struct MAP_PREFIX(Entry) {
    MAP_KEY_TYPE key;
    MAP_VALUE_TYPE value;
    struct MAP_PREFIX(Entry) *next; 
};

void MAP_PREFIX(init)(
    struct MAP_TYPE *const self,
    usize const table_size
) {
    self->table_size    = table_size;
    self->element_count = 0u;
    self->entries       = calloc(sizeof (void *), self->table_size);
}

void MAP_PREFIX(clone)(
    struct MAP_TYPE *const self,
    struct MAP_TYPE const *const other
) {
    self->table_size    = other->table_size;
    self->element_count = other->element_count;
    self->entries       = calloc(sizeof (void *), self->table_size);

    memcpy(self->entries, other->entries, sizeof (void *) * other->table_size);
}

void MAP_PREFIX(free)(
    struct MAP_TYPE *const self
) {
    for (usize entry_index = 0u; entry_index < self->table_size; ++entry_index) {
        struct MAP_PREFIX(Entry) *entry = (struct MAP_PREFIX(Entry) *) self->entries[entry_index];

        while (entry != NULL) {
            struct MAP_PREFIX(Entry) *const next = entry->next;
            free(entry);
            entry = next;
        }
    }
    
    free(self->entries);
}

MAP_VALUE_TYPE *MAP_PREFIX(get)(
    struct MAP_TYPE const *const self,
    MAP_KEY_TYPE const key
) {
    usize const entry_index = MAP_KEY_HASH_FN(key) % self->table_size;
    struct MAP_PREFIX(Entry) *entry = (struct MAP_PREFIX(Entry) *) self->entries[entry_index];

    // find first entry where key matches
    while (entry != NULL) {
        if (MAP_KEY_EQ_FN(key, entry->key)) {
            return &entry->value;
        }

        entry = entry->next;
    }

    // not found
    return NULL;
}

bool MAP_PREFIX(contains_key)(
    struct MAP_TYPE const *const self,
    MAP_KEY_TYPE const key
) {
    usize const entry_index = MAP_KEY_HASH_FN(key) % self->table_size;
    struct MAP_PREFIX(Entry) *entry = (struct MAP_PREFIX(Entry) *) self->entries[entry_index];

    while (entry != NULL) {
        if (MAP_KEY_EQ_FN(key, entry->key)) {
            return true;
        }

        entry = entry->next;
    }

    return false;
}

void MAP_PREFIX(set)(
    struct MAP_TYPE *const self, 
    MAP_KEY_TYPE const key, 
    MAP_VALUE_TYPE const value
) {
    usize const entry_index = MAP_KEY_HASH_FN(key) % self->table_size;
    struct MAP_PREFIX(Entry) **entry = (struct MAP_PREFIX(Entry) **) &self->entries[entry_index];

    // find first entry where key matches
    while (*entry != NULL) {
        if (MAP_KEY_EQ_FN(key, (*entry)->key)) {
            break;
        }

        entry = &(*entry)->next;
    }

    // if no entry exists, create one
    if (*entry == NULL) {
        *entry = malloc(sizeof (struct MAP_PREFIX(Entry)));
        (*entry)->key = key;
        (*entry)->next = NULL;
    }

    (*entry)->value = value;
}

bool MAP_PREFIX(remove)(
    struct MAP_TYPE *const self,
    MAP_KEY_TYPE const key
) {
    usize const entry_index = MAP_KEY_HASH_FN(key) % self->table_size;
    struct MAP_PREFIX(Entry) **entry = (struct MAP_PREFIX(Entry) **) &self->entries[entry_index];

    // find first entry where key matches
    while (*entry != NULL) {
        if (MAP_KEY_EQ_FN(key, (*entry)->key)) {
            break;
        }

        entry = &(*entry)->next;
    }

    if (*entry != NULL) {
        struct MAP_PREFIX(Entry) *const next = (*entry)->next;
        free(*entry);
        *entry = next;
        return true;
    } else {
        return false;
    }
}

#undef MAP_CONCAT1
#undef MAP_CONCAT2
#undef MAP_PREFIX
