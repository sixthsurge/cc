#pragma once 

#include "cc/hash.h"
#include "cc/slice.h"

#define MAP_TYPE            Map__CharSlice_usize
#define MAP_KEY_TYPE        struct CharSlice 
#define MAP_VALUE_TYPE      usize
#define MAP_FUNCTION_PREFIX map__charslice_usize__
#define MAP_KEY_EQ_FN       charslice_eq
#define MAP_KEY_HASH_FN     charslice_hash_djb2
#include "cc/template/map.h"
#undef MAP_TYPE            
#undef MAP_KEY_TYPE 
#undef MAP_VALUE_TYPE      
#undef MAP_FUNCTION_PREFIX 
#undef MAP_KEY_EQ_FN       
#undef MAP_KEY_HASH_FN     
