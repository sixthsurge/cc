#include "cc/compile/function_table.h"
#include "cc/compile/error.h"
#include "cc/compile/function_signature.h"
#include "cc/map.h"

// define FunctionDescriptionSlice and FunctionDescriptionVec
#define SLICE_TYPE FunctionDescriptionSlice 
#define SLICE_ELEMENT_TYPE struct FunctionDescription 
#define SLICE_FUNCTION_PREFIX fdslice_
#include "cc/template/slice.inl"
#undef SLICE_TYPE
#undef SLICE_ELEMENT_TYPE
#undef SLICE_FUNCTION_PREFIX

#define VEC_TYPE FunctionDescriptionVec 
#define VEC_ELEMENT_TYPE struct FunctionDescription 
#define VEC_SLICE_TYPE FunctionDescriptionSlice
#define VEC_FUNCTION_PREFIX fdvec_
#include "cc/template/vec.inl"
#undef VEC_ELEMENT_TYPE
#undef VEC_TYPE
#undef VEC_SLICE_TYPE
#undef VEC_FUNCTION_PREFIX


#define FUNCTION_INDEX_SIZE 1021u

static void function_table_set(
    struct FunctionTable *const self, 
    struct FunctionDescription const function_desc
) {
    usize const *const existing_index = map__charslice_usize__get(
        &self->function_index, 
        function_desc.name
    );

    if (existing_index != NULL) {
        *fdvec_at(&self->function_descriptions, *existing_index) = function_desc;
    } else {
        map__charslice_usize__set(
            &self->function_index,
            function_desc.name, 
            self->function_descriptions.len
        );
        fdvec_push(&self->function_descriptions, function_desc);
    }
}

void function_table_init(struct FunctionTable *const self) {
    map__charslice_usize__init(&self->function_index, FUNCTION_INDEX_SIZE);
    fdvec_init(&self->function_descriptions);
}

void function_table_free(struct FunctionTable *const self) {
    // free function signatures 
    for (usize i = 0; i < self->function_descriptions.len; i += 1) {
        function_signature_free(&fdvec_at(&self->function_descriptions, i)->signature);
    }

    map__charslice_usize__free(&self->function_index);
    fdvec_free(&self->function_descriptions);
}

bool function_table_has(struct FunctionTable *const self, struct CharSlice const name) {
    return map__charslice_usize__get(&self->function_index, name) != NULL;
}

bool function_table_get(
    struct FunctionTable const *const self, 
    struct CharSlice const name, 
    struct FunctionDescription *const out
) {
    usize const *const index = map__charslice_usize__get(&self->function_index, name);

    if (index != NULL) {
        return fdvec_at(&self->function_descriptions, *index);
    } else {
        return false;
    }
}

struct CompileResult function_table_declare(
    struct FunctionTable *self, 
    struct CharSlice name, 
    struct FunctionSignature const *signature
) {
    struct FunctionDescription existing_function_desc;

    if (function_table_get(self, name, &existing_function_desc)) {
        if (!function_signatures_match(&existing_function_desc.signature, signature)) {
            return compile_error(
                (struct CompileError) {
                    .kind = CompileErrorFunctionSignatureMismatch,
                    .function_signature_mismatch = {
                        .name = name,
                    },
                }
            );
        }
    } else {
        struct FunctionDescription const function_desc = {
            .name = name, 
            .signature = function_signature_clone(signature),
            .has_definition = true,
        };
        function_table_set(self, function_desc);
    }

    return compile_ok();
}

struct CompileResult function_table_define(
    struct FunctionTable *self, 
    struct CharSlice name, 
    struct FunctionSignature const *signature
) {
    struct FunctionDescription existing_function_desc;

    if (function_table_get(self, name, &existing_function_desc)) {
        if (!function_signatures_match(&existing_function_desc.signature, signature)) {
            return compile_error(
                (struct CompileError) {
                    .kind = CompileErrorFunctionSignatureMismatch,
                    .function_signature_mismatch = {
                        .name = name,
                    },
                }
            );
        } else if (existing_function_desc.has_definition) {
            return compile_error(
                (struct CompileError) {
                    .kind = CompileErrorFunctionRedefinition,
                    .function_signature_mismatch = {
                        .name = name,
                    },
                }
            );
        }

        usize const index = *map__charslice_usize__get(&self->function_index, name);
        fdvec_at(&self->function_descriptions, index)->has_definition = true;
    } else {
        struct FunctionDescription const function_desc = {
            .name = name, 
            .signature = function_signature_clone(signature),
            .has_definition = true,
        };
        function_table_set(self, function_desc);
    }

    return compile_ok();
}
