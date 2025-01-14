#include "assembly.h"

#include <stdlib.h>

#include "log.h"
#include "writer.h"

char const *format_operand_width(
    enum OperandWidth const operand_width
) {
    if (operand_width >= OperandWidthCount) {
        log_error("invalid operand width: %zu", (usize) operand_width);
        exit(1);
    }

    char const *const names[4] = {
        "byte",
        "word",
        "dword",
        "qword"
    };

    return names[(usize) operand_width];
}

char const *format_register(
    enum Register const reg,
    enum OperandWidth const operand_width
) {
    if (reg >= RegisterCount) {
        log_error("invalid register: %zu", (usize) reg);
        exit(1);
    }
    if (operand_width >= OperandWidthCount) {
        log_error("invalid operand width: %zu", (usize) operand_width);
        exit(1);
    }

    char const *const byte_names[RegisterCount] = {
        "al",
        "bl",
        "cl",
        "dl",
        "sil",
        "dil",
        "spl",
        "bpl",
        "r8b",
        "r9b",
        "r10b",
        "r11b",
        "r12b",
        "r13b",
        "r14b",
        "r15b"
    };

    char const *const word_names[RegisterCount] = {
        "ax",
        "bx",
        "cx",
        "dx",
        "si",
        "di",
        "sp",
        "bp",
        "r8d",
        "r9d",
        "r10w",
        "r11w",
        "r12w",
        "r13w",
        "r14w",
        "r15w"
    };

    char const *const dword_names[RegisterCount] = {
        "eax",
        "ebx",
        "ecx",
        "edx",
        "esi",
        "edi",
        "esp",
        "ebp",
        "r8d",
        "r9d",
        "r10d",
        "r11d",
        "r12d",
        "r13d",
        "r14d",
        "r15d"
    };

    char const *const qword_names[RegisterCount] = {
        "rax",
        "rbx",
        "rcx",
        "rdx",
        "rsi",
        "rdi",
        "rsp",
        "rbp",
        "r8",
        "r9",
        "r10",
        "r11",
        "r12",
        "r13",
        "r14",
        "r15"
    };

    switch (operand_width) {
        case Byte: 
            return byte_names[reg];
        case Word: 
            return word_names[reg];
        case DWord: 
            return dword_names[reg];
        case QWord: 
            return qword_names[reg];
        default:
            return "??";
    }
}

char const *format_instruction(enum Instruction instruction) {
    if (instruction >= InstructionCount) {
        log_error("invalid instruction: %zu", (usize) instruction);
        exit(1);
    }

    char const *const names[InstructionCount] = {
        "leave",
        "ret",
        "push",
        "pop",
        "mov",
        "add",
        "sub",
    };

    return names[instruction];
}

usize instruction_expected_operand_count(enum Instruction instruction) {
    if (instruction >= InstructionCount) {
        log_error("invalid instruction: %zu", (usize) instruction);
        exit(1);
    }

    usize counts[InstructionCount] = {
        0u,
        0u,
        1u,
        1u,
        2u,
        2u,
        2u,
    };

    return counts[instruction];
}

struct Operand operand_immediate(enum OperandWidth const width, i32 const value) {
    return (struct Operand) {
        .kind = OperandImmediate,
        .width = width,
        .immediate = {
            .value = value,
        },
    };
}

struct Operand operand_register(enum OperandWidth const width, enum Register const reg) {
    return (struct Operand) {
        .kind = OperandRegister,
        .width = width,
        .reg = {
            .reg = reg,
        },
    };
}

struct Operand operand_memory(
    enum OperandWidth const width, 
    enum Register const base_reg, 
    i32 const offset
) {
    return (struct Operand) {
        .kind = OperandMemory,
        .width = width,
        .memory = {
            .base_reg = base_reg,
            .offset = offset,
        },
    };
}

struct Operand operand_memory_indexed(
    enum OperandWidth const width, 
    enum Register const base_reg, 
    enum Register const index_reg, 
    i32 const offset, 
    i32 const index_scale
) {
    return (struct Operand) {
        .kind = OperandMemoryIndexed,
        .width = width,
        .memory_indexed = {
            .base_reg = base_reg,
            .index_reg = index_reg,
            .offset = offset,
            .index_scale = index_scale,
        },
    };
}

struct Operand operand_stack(enum OperandWidth const width, usize const stack_offset) {
    return (struct Operand) {
        .kind = OperandMemory,
        .width = width,
        .memory = {
            .base_reg = RegisterBP,
            .offset = -((i32) stack_offset),
        },
    };
}

void write_operand(struct Writer *const assembly_writer, struct Operand const operand) {
    switch (operand.kind) {
        case OperandImmediate: {
            writer_writef(
                assembly_writer,
                "%d",
                operand.immediate.value
            );
            break;
        }
        case OperandRegister: {
            writer_write(
                assembly_writer,
                format_register(operand.reg.reg, operand.width)
            );
            break;
        }
        case OperandMemory: {
            writer_writef(
                assembly_writer,
                "%s [%s%+d]",
                format_operand_width(operand.width),
                format_register(operand.memory.base_reg, QWord),
                operand.memory.offset
            );
            break;
        }
        case OperandMemoryIndexed: {
            writer_writef(
                assembly_writer,
                "%s [%s+%s*%d%+d]",
                format_operand_width(operand.width),
                format_register(operand.memory_indexed.base_reg, QWord),
                format_register(operand.memory_indexed.index_reg, QWord),
                operand.memory_indexed.index_scale,
                operand.memory_indexed.offset
            );
            break;
        }
    }
}

void write_instruction(
    struct Writer *const assembly_writer, 
    enum Instruction const instruction,
    usize const operand_count,
    ...
) {
    if (operand_count != instruction_expected_operand_count(instruction)) {
        log_error(
            "write_instruction: instruction %s; expected %zu operands, got %zu",
            format_instruction(instruction),
            instruction_expected_operand_count(instruction),
            operand_count
        );
        exit(1);
    }

    writer_write(assembly_writer, "\t");
    writer_write(assembly_writer, format_instruction(instruction));

    va_list operands;
    va_start(operands, operand_count);

    for (usize operand_index = 0u; operand_index < operand_count; operand_index += 1) {
        struct Operand const operand = va_arg(operands, struct Operand);

        writer_write(assembly_writer, " ");
        write_operand(assembly_writer, operand);

        if (operand_index < operand_count - 1) {
            writer_write(assembly_writer, ",");
        }
    }

    va_end(operands);

    writer_write(assembly_writer, "\n");
}
