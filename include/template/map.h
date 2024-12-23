// define 
// MAP_TYPE            | Name for map type e.g. StringIntMap
// MAP_KEY_TYPE        | Type of keys e.g. char const *
// MAP_VALUE_TYPE      | Type of values e.g. int
// MAP_FUNCTION_PREFIX | Prefix for functions on the map, e.g. stringintmap_
// MAP_KEY_HASH_FN     | Hash function for keys   (MAP_KEY_TYPE -> usize)
// MAP_KEY_EQ_FN       | Hash function for values (MAP_KEY_TYPE, MAP_KEY_TYPE -> bool)

#include "common.h"

#define MAP_CONCAT1(a, b) a##b
#define MAP_CONCAT2(a, b) MAP_CONCAT1(a, b)
#define MAP_PREFIX(function) MAP_CONCAT2(MAP_FUNCTION_PREFIX, function)

typedef struct MAP_TYPE {
    // Number of entries in the hash table
    usize table_size;
    // Number of key/value pairs in the map
    usize element_count;
    // Hash table of entries 
    void **entries;
} MAP_TYPE;

void            MAP_PREFIX(init)  (MAP_TYPE *self, usize table_size);
void            MAP_PREFIX(clone) (MAP_TYPE *self, MAP_TYPE const *other);
void            MAP_PREFIX(free)  (MAP_TYPE *self);
MAP_VALUE_TYPE *MAP_PREFIX(get)   (MAP_TYPE const *self, MAP_KEY_TYPE key);
void            MAP_PREFIX(set)   (MAP_TYPE *self, MAP_KEY_TYPE key, MAP_VALUE_TYPE value);
bool            MAP_PREFIX(remove)(MAP_TYPE *self, MAP_KEY_TYPE key);

#undef MAP_CONCAT1
#undef MAP_CONCAT2
#undef MAP_PREFIX
