#include "cc/compile/assembly.h"

#include <stdlib.h>

#include "cc/common.h"
#include "cc/log.h"
#include "cc/writer.h"

static bool operand_eq(struct Operand const *const left, struct Operand const *const right) {
    if (left->kind != right->kind) return false;

    switch (left->kind) {
    case OperandImmediate:
        return left->variant.immediate.value == right->variant.immediate.value;
    case OperandRegister:
        return left->variant.int_register.reg == right->variant.int_register.reg;
    case OperandMemory:
        return left->variant.memory.base_reg == right->variant.memory.base_reg
            && left->variant.memory.offset == right->variant.memory.offset;
    case OperandMemoryIndexed:
        return left->variant.memory_indexed.base_reg == right->variant.memory_indexed.base_reg
            && left->variant.memory_indexed.index_reg == right->variant.memory_indexed.index_reg
            && left->variant.memory_indexed.offset == right->variant.memory_indexed.offset
            && left->variant.memory_indexed.index_scale == right->variant.memory_indexed.index_scale;
    }

    log_error("operand_eq: unknown operand kind %zu", (usize) left->kind);
    exit(1);
}

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
    enum IntRegister const reg,
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
        "cdq",
        "push",
        "pop",
        "call",
        "idiv",
        "mov",
        "add",
        "sub",
        "imul",
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
        0u,
        1u,
        1u,
        1u,
        1u,
        2u,
        2u,
        2u,
        2u,
    };

    return counts[instruction];
}

struct Operand operand_immediate(i32 const value) {
    return (struct Operand) {
        .kind = OperandImmediate,
        .variant.immediate = {
            .value = value,
        },
    };
}

struct Operand operand_label(struct CharSlice const name) {
    return (struct Operand) {
        .kind = OperandLabel,
        .variant.label = {
            .name = name,
        },
    };
}

struct Operand operand_register(enum IntRegister const reg) {
    return (struct Operand) {
        .kind = OperandRegister,
        .variant.int_register = {
            .reg = reg,
        },
    };
}

struct Operand operand_memory(enum IntRegister const base_reg, i32 const offset) {
    return (struct Operand) {
        .kind = OperandMemory,
        .variant.memory = {
            .base_reg = base_reg,
            .offset = offset,
        },
    };
}

struct Operand operand_memory_indexed(
    enum IntRegister const base_reg, 
    enum IntRegister const index_reg, 
    i32 const offset, 
    i32 const index_scale
) {
    return (struct Operand) {
        .kind = OperandMemoryIndexed,
        .variant.memory_indexed = {
            .base_reg = base_reg,
            .index_reg = index_reg,
            .offset = offset,
            .index_scale = index_scale,
        },
    };
}

struct Operand operand_stack(usize const stack_offset) {
    return (struct Operand) {
        .kind = OperandMemory,
        .variant.memory = {
            .base_reg = RegisterBP,
            .offset = -((i32) stack_offset),
        },
    };
}

void emit_operand(
    struct Writer *const assembly_writer, 
    struct Operand const operand, 
    enum OperandWidth const width
) {
    switch (operand.kind) {
        case OperandImmediate: {
            writer_writef(
                assembly_writer,
                "%d",
                operand.variant.immediate.value
            );
            break;
        }
        case OperandLabel: {
            writer_write_charslice(
                assembly_writer,
                operand.variant.label.name
            );
            break;
        }
        case OperandRegister: {
            writer_write(
                assembly_writer,
                format_register(operand.variant.int_register.reg, width)
            );
            break;
        }
        case OperandMemory: {
            writer_writef(
                assembly_writer,
                "%s [%s%+d]",
                format_operand_width(width),
                format_register(operand.variant.memory.base_reg, QWord),
                operand.variant.memory.offset
            );
            break;
        }
        case OperandMemoryIndexed: {
            writer_writef(
                assembly_writer,
                "%s [%s+%s*%d%+d]",
                format_operand_width(width),
                format_register(operand.variant.memory_indexed.base_reg, QWord),
                format_register(operand.variant.memory_indexed.index_reg, QWord),
                operand.variant.memory_indexed.index_scale,
                operand.variant.memory_indexed.offset
            );
            break;
        }
    }
}

void emit_instruction(
    struct Writer *const assembly_writer, 
    enum Instruction const instruction,
    enum OperandWidth const operand_width,
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
        emit_operand(assembly_writer, operand, operand_width);

        if (operand_index < operand_count - 1) {
            writer_write(assembly_writer, ",");
        }
    }

    va_end(operands);

    writer_write(assembly_writer, "\n");
}

void emit_label(struct Writer *const assembly_writer, struct CharSlice const label) {
    writer_write_charslice(assembly_writer, label);
    writer_write(assembly_writer, ":\n");
}

void emit_function_prologue(struct Writer *const assembly_writer, usize const stack_usage) {
    emit_instruction(
        assembly_writer, 
        InstructionPush, 
        QWord,
        1u, 
        operand_register(RegisterBP)
    );
    emit_instruction(
        assembly_writer, 
        InstructionMov, 
        QWord,
        2u, 
        operand_register(RegisterBP),
        operand_register(RegisterSP)
    );
    if (stack_usage > 0u) {
        emit_instruction(
            assembly_writer, 
            InstructionSub, 
            QWord,
            2u, 
            operand_register(RegisterSP),
            operand_immediate(round_up_usize(stack_usage, 16u))
        );
    }
}

void emit_function_exit(struct Writer *assembly_writer) {
    emit_instruction(
        assembly_writer, 
        InstructionLeave, 
        QWord,
        0u
    );
    emit_instruction(
        assembly_writer, 
        InstructionRet, 
        QWord,
        0u
    );
}

void emit_moves(
    struct Writer *const assembly_writer, 
    struct Operand const dst, 
    struct Operand const src,
    enum OperandWidth const operand_width,
    enum IntRegister const intermediate_register
) {
    if (dst.kind == OperandImmediate) {
        log_error("emit_moves: dst cannot be immediate");
        exit(1);
    }

    if (operand_eq(&src, &dst)) {
        // nop
    } else if (
        dst.kind == OperandRegister || src.kind == OperandRegister || src.kind == OperandImmediate
    ) {
        // src -> dst
        emit_instruction(
            assembly_writer,
            InstructionMov,
            operand_width,
            2u,
            dst,
            src
        );
    } else {
        // src -> intermediate_register -> dst
        struct Operand const intermediate_location 
            = operand_register(intermediate_register);

        emit_instruction(
            assembly_writer,
            InstructionMov,
            operand_width,
            2u,
            intermediate_location,
            src
        );
        emit_instruction(
            assembly_writer,
            InstructionMov,
            operand_width,
            2u,
            dst,
            intermediate_location
        );
    }
}

void emit_assign_variable(
    struct Writer *const assembly_writer,
    usize const assignee_stack_offset,
    struct Type const assignee_type,
    struct Operand const assigned_location,
    struct Type const assigned_type
) {
    // TODO: support types not 4 bytes long
    emit_moves(
        assembly_writer,
        operand_stack(assignee_stack_offset),
        assigned_location,
        DWord,
        RegisterA
    );
}
