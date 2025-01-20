#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef size_t   usize;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    f32;
typedef double   f64;

static char const *const color_reset   = "\033[0m";
static char const *const color_black   = "\033[0;30m";
static char const *const color_red     = "\033[0;31m";
static char const *const color_green   = "\033[0;32m";
static char const *const color_yellow  = "\033[0;33m";
static char const *const color_magenta = "\033[0;35m";

static inline usize max_usize(usize const a, usize const b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

static inline usize round_up_usize(usize value, usize round) {
    usize const floor = (value / round) * round;

    if (value == floor) {
        return value;
    } else {
        return floor + round;
    }
}

#ifdef __GNUC__
    #define LIKELY(COND)   __builtin_expect(!!(COND), 1)
    #define UNLIKELY(COND) __builtin_expect(!!(COND), 0)
    #define NORETURN       __attribute__((noreturn))

    #define ATTRIBUTE_PRINTF_LIKE(FORMAT_STRING_INDEX, ARGS_INDEX) __attribute__((__format__ (__printf__, FORMAT_STRING_INDEX, ARGS_INDEX)))
#else
    #define LIKELY(COND)   COND
    #define UNLIKELY(COND) COND
    #define NORETURN

    #define ATTATTRIBUTE_PRINTF_LIKE(FORMAT_STRING_INDEX, ARGS_INDEX)
#endif

