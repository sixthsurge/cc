// define 
// SLICE_TYPE            | Name for slice type e.g. CharSlice
// SLICE_ELEMENT_TYPE    | Type of slice elements e.g. char
// SLICE_FUNCTION_PREFIX | Prefix for functions on the slice, e.g. charslice_
// SLICE_DEBUG_FN        | (Optional) function to print an element of the slice

#define SLICE_CONCAT1(a, b) a##b
#define SLICE_CONCAT2(a, b) SLICE_CONCAT1(a, b)
#define SLICE_PREFIX(function) SLICE_CONCAT2(SLICE_FUNCTION_PREFIX, function)

#ifdef SLICE_DEBUG_FN
void SLICE_PREFIX(debug)(struct Writer *writer, struct SLICE_TYPE const *self) {
    writer_write(writer, "[");
    for (usize i = 0; i < self->len; ++i) {
        SLICE_DEBUG_FN(writer, &self->ptr[i]);

        if (i < self->len - 1) {
            writer_write(writer, ", ");
        }
    }
    writer_write(writer, "]");
}
#endif

#undef SLICE_CONCAT1
#undef SLICE_CONCAT2
#undef SLICE_PREFIX
