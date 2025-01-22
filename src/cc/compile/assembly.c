#include "cc/compile/assembly.h"

#include <stdlib.h>

#include "cc/common.h"
#include "cc/integer_size.h"
#include "cc/log.h"
#include "cc/type.h"
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
            && left->variant.memory.displacement == right->variant.memory.displacement;
    case OperandMemoryIndexed:
        return left->variant.memory_indexed.base_reg == right->variant.memory_indexed.base_reg
            && left->variant.memory_indexed.index_reg == right->variant.memory_indexed.index_reg
            && left->variant.memory_indexed.displacement == right->variant.memory_indexed.displacement
            && left->variant.memory_indexed.index_scale == right->variant.memory_indexed.index_scale;
    case OperandLabel:
        return charslice_eq(left->variant.label.name, right->variant.label.name);
    }

    log_error("operand_eq: unknown operand kind %zu", (usize) left->kind);
    exit(1);
}

static enum OperandWidth operand_width_with_size(usize const size) {
    if (size <= 1u) {
        return Byte;
    } else if (size <= 2u) {
        return Word;
    } else if (size <= 4u) {
        return DWord;
    } else if (size <= 8u) {
        return QWord;
    } else {
        log_error("operand_width_with_size: size %zu is too large", size);
        exit(1);
    }
}

enum OperandWidth integer_operand_width(enum IntegerSize size) {
    return operand_width_with_size(integer_size_bytes(size));
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
        "cdqe",
        "push",
        "pop",
        "call",
        "idiv",
        "mov",
        "movsx",
        "movzx",
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
        0u,
        1u,
        1u,
        1u,
        1u,
        2u,
        2u,
        2u,
        2u,
        2u,
        2u,
    };

    return counts[instruction];
}

struct Operand operand_immediate(u64 const value) {
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

struct Operand operand_memory(enum IntRegister const base_reg, i64 const displacement) {
    return (struct Operand) {
        .kind = OperandMemory,
        .variant.memory = {
            .base_reg = base_reg,
            .displacement = displacement,
        },
    };
}

struct Operand operand_memory_indexed(
    enum IntRegister const base_reg, 
    enum IntRegister const index_reg, 
    i64 const displacement, 
    i64 const index_scale
) {
    return (struct Operand) {
        .kind = OperandMemoryIndexed,
        .variant.memory_indexed = {
            .base_reg = base_reg,
            .index_reg = index_reg,
            .displacement = displacement,
            .index_scale = index_scale,
        },
    };
}

struct Operand operand_stack(usize const stack_offset) {
    return (struct Operand) {
        .kind = OperandMemory,
        .variant.memory = {
            .base_reg = RegisterBP,
            .displacement = -((i32) stack_offset),
        },
    };
}

struct Operand operand_displace(struct Operand operand, i64 const amount_bytes) {
    switch (operand.kind) {
        case OperandMemory: {
            operand.variant.memory.displacement += amount_bytes;
            return operand;
        }
        case OperandMemoryIndexed: {
            operand.variant.memory_indexed.displacement += amount_bytes;
            return operand;
        }
        default: {
            log_error(
                "operand_displace: cannot displace operand of kind %zu", 
                (usize) operand.kind
            );
            exit(1);
        }
    }
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
                "%lu",
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
                "%s [%s%+ld]",
                format_operand_width(width),
                format_register(operand.variant.memory.base_reg, QWord),
                operand.variant.memory.displacement
            );
            break;
        }
        case OperandMemoryIndexed: {
            writer_writef(
                assembly_writer,
                "%s [%s+%s*%ld%+ld]",
                format_operand_width(width),
                format_register(operand.variant.memory_indexed.base_reg, QWord),
                format_register(operand.variant.memory_indexed.index_reg, QWord),
                operand.variant.memory_indexed.index_scale,
                operand.variant.memory_indexed.displacement
            );
            break;
        }
    }
}

void emit_instruction(
    struct Writer *const assembly_writer, 
    enum Instruction const instruction
) {
    if (instruction_expected_operand_count(instruction) != 0) {
        log_error(
            "emit_instruction: instruction %s expected %zu operand(s)",
            format_instruction(instruction),
            instruction_expected_operand_count(instruction)
        );
        exit(1);
    }

    writer_write(assembly_writer, "\t");
    writer_write(assembly_writer, format_instruction(instruction));
    writer_write(assembly_writer, "\n");
}

void emit_instruction_single_operand(
    struct Writer *const assembly_writer, 
    enum Instruction const instruction, 
    enum OperandWidth const operand_width,
    struct Operand const operand
) {
    if (instruction_expected_operand_count(instruction) != 1) {
        log_error(
            "emit_instruction_single_operand: instruction %s requires %zu operand(s)",
            format_instruction(instruction),
            instruction_expected_operand_count(instruction)
        );
        exit(1);
    }

    writer_write(assembly_writer, "\t");
    writer_write(assembly_writer, format_instruction(instruction));
    writer_write(assembly_writer, " ");
    emit_operand(assembly_writer, operand, operand_width);
    writer_write(assembly_writer, "\n");
}

void emit_instruction_dst_src(
    struct Writer *const assembly_writer, 
    enum Instruction const instruction, 
    enum OperandWidth const dst_width,
    enum OperandWidth const src_width,
    struct Operand const dst,
    struct Operand const src
) {
    if (instruction_expected_operand_count(instruction) != 2) {
        log_error(
            "emit_instruction_dst_src: instruction %s requires %zu operand(s)",
            format_instruction(instruction),
            instruction_expected_operand_count(instruction)
        );
        exit(1);
    }

    writer_write(assembly_writer, "\t");
    writer_write(assembly_writer, format_instruction(instruction));
    writer_write(assembly_writer, " ");
    emit_operand(assembly_writer, dst, dst_width);
    writer_write(assembly_writer, ", ");
    emit_operand(assembly_writer, src, src_width);
    writer_write(assembly_writer, "\n");
}

void emit_label(struct Writer *const assembly_writer, struct CharSlice const label) {
    writer_write_charslice(assembly_writer, label);
    writer_write(assembly_writer, ":\n");
}

void emit_function_prologue(struct Writer *const assembly_writer, usize const stack_usage) {
    emit_instruction_single_operand(
        assembly_writer, 
        InstructionPush, 
        QWord,
        operand_register(RegisterBP)
    );
    emit_instruction_dst_src(
        assembly_writer, 
        InstructionMov, 
        QWord,
        QWord,
        operand_register(RegisterBP),
        operand_register(RegisterSP)
    );
    if (stack_usage > 0u) {
        emit_instruction_dst_src(
            assembly_writer, 
            InstructionSub, 
            QWord,
            QWord, 
            operand_register(RegisterSP),
            operand_immediate(round_up_usize(stack_usage, 16u))
        );
    }
}

void emit_function_exit(struct Writer *assembly_writer) {
    emit_instruction(assembly_writer, InstructionLeave);
    emit_instruction(assembly_writer, InstructionRet);
}

void emit_move(
    struct Writer *const assembly_writer, 
    struct Operand const dst, 
    struct Operand const src,
    enum OperandWidth const dst_operand_width,
    enum OperandWidth const src_operand_width,
    enum IntRegister const intermediate_register
) {
    if (dst.kind == OperandImmediate || dst.kind == OperandLabel) {
        log_error("emit_moves: dst cannot be immediate or label");
        exit(1);
    }

    if (operand_eq(&src, &dst)) {
        // nop
    } else if (
        dst.kind == OperandRegister || src.kind == OperandRegister || src.kind == OperandImmediate
    ) {
        // src -> dst
        emit_instruction_dst_src(
            assembly_writer,
            InstructionMov,
            dst_operand_width,
            dst_operand_width,
            dst,
            src
        );
    } else {
        // src -> intermediate_register -> dst
        struct Operand const intermediate_location 
            = operand_register(intermediate_register);

        emit_instruction_dst_src(
            assembly_writer,
            InstructionMov,
            src_operand_width,
            src_operand_width,
            intermediate_location,
            src
        );
        emit_instruction_dst_src(
            assembly_writer,
            InstructionMov,
            dst_operand_width,
            dst_operand_width,
            dst,
            intermediate_location
        );
    }
}

void emit_move_bytes(
    struct Writer *const assembly_writer, 
    struct Operand const dst, 
    struct Operand const src, 
    usize const size_bytes,
    enum IntRegister const intermediate_register
) {
    if (size_bytes <= 8u) {
        enum OperandWidth const operand_width = operand_width_with_size(size_bytes);

        emit_move(
            assembly_writer, 
            dst, 
            src, 
            operand_width, 
            operand_width, 
            intermediate_register
        );
    } else {
        if (dst.kind == OperandRegister) {
            log_error("emit_move_bytes: %zu bytes is too many to fit in a register", size_bytes);
            exit(1);
        }

        // copy bytes from src to dst

        struct Operand operand_dst = dst;
        struct Operand operand_src = src;
        i64 bytes_remaining = (i64) size_bytes;

        // move qwords
        while (bytes_remaining >= 8) {
            emit_move(
                assembly_writer, 
                operand_dst, 
                operand_src,
                QWord,
                QWord,
                intermediate_register
            );

            bytes_remaining -= 8;
            operand_dst = operand_displace(operand_dst, 8);
            operand_src = operand_displace(operand_src, 8);
        }

        // last dword
        if (bytes_remaining >= 4) {
            emit_move(
                assembly_writer, 
                operand_dst, 
                operand_src,
                DWord,
                DWord,
                intermediate_register
            );

            bytes_remaining -= 4;
            operand_dst = operand_displace(operand_dst, 4);
            operand_src = operand_displace(operand_src, 4);
        }

        // last word 
        if (bytes_remaining >= 2) {
            emit_move(
                assembly_writer, 
                operand_dst, 
                operand_src,
                Word,
                Word,
                intermediate_register
            );

            bytes_remaining -= 2;
            operand_dst = operand_displace(operand_dst, 2);
            operand_src = operand_displace(operand_src, 2);
        }

        // last byte
        if (bytes_remaining >= 1) {
            emit_move(
                assembly_writer, 
                operand_dst, 
                operand_src,
                Byte,
                Byte,
                intermediate_register
            );

            bytes_remaining -= 1;
            operand_dst = operand_displace(operand_dst, 1);
            operand_src = operand_displace(operand_src, 1);
        }
    }
}

void emit_assignment(
    struct Writer *const assembly_writer,
    struct Operand const dst,
    struct Operand const src,
    struct Type const dst_type,
    struct Type const src_type
) {
    if (dst.kind == OperandImmediate || dst.kind == OperandLabel) {
        log_error("emit_assignment: dst cannot be immediate or label");
        exit(1);
    }

    usize const dst_size = type_size_bytes(&dst_type);

    if (type_eq(&dst_type, &src_type)) {
        emit_move_bytes(
            assembly_writer, 
            dst, 
            src, 
            dst_size,
            RegisterA
        );
    } else if (dst_type.kind == TypeInteger && src_type.kind == TypeInteger) {
        enum IntegerSize const dst_size = dst_type.variant.integer_type.size;
        enum IntegerSize const src_size = src_type.variant.integer_type.size;
        enum OperandWidth const dst_width = integer_operand_width(dst_size);
        enum OperandWidth const src_width = integer_operand_width(src_size);
        bool const src_signed = src_type.variant.integer_type.is_signed;

        bool const is_nop 
            = integer_size_bytes(src_size) >= integer_size_bytes(dst_size)
            || (dst_size == IntegerSize64 && src_size == IntegerSize32 && !src_signed);

        if (is_nop) {
            emit_move(
                assembly_writer, 
                dst, 
                src, 
                dst_width, 
                src_width, 
                RegisterA
            );
        } else if (dst_size == IntegerSize64 && src_size == IntegerSize32 && src_signed) {
            // cdqe

            emit_move(
                assembly_writer,
                operand_register(RegisterA),
                src,
                src_width,
                src_width,
                RegisterA
            );
            emit_instruction(
                assembly_writer, 
                InstructionCdqe
            );
            emit_move(
                assembly_writer,
                dst,
                operand_register(RegisterA),
                dst_width,
                dst_width,
                RegisterA
            );
        } else if (src_signed) {
            // movsx

            if (dst.kind == OperandRegister) {
                emit_instruction_dst_src(
                    assembly_writer, 
                    InstructionMovSx, 
                    dst_width, 
                    src_width, 
                    dst,
                    src
                );
            } else {
                emit_instruction_dst_src(
                    assembly_writer, 
                    InstructionMovSx, 
                    dst_width, 
                    src_width, 
                    operand_register(RegisterA), 
                    src
                );
                emit_move(
                    assembly_writer,
                    dst,
                    operand_register(RegisterA),
                    dst_width,
                    dst_width,
                    RegisterA
                );
            }
        } else {
            // movzx

            if (dst.kind == OperandRegister) {
                emit_instruction_dst_src(
                    assembly_writer, 
                    InstructionMovZx, 
                    dst_width, 
                    src_width, 
                    dst,
                    src
                );
            } else {
                emit_instruction_dst_src(
                    assembly_writer, 
                    InstructionMovZx, 
                    dst_width, 
                    src_width, 
                    operand_register(RegisterA), 
                    src
                );
                emit_move(
                    assembly_writer,
                    dst,
                    operand_register(RegisterA),
                    dst_width,
                    dst_width,
                    RegisterA
                );
            }
        }
    } else {
        log_error(
            "emit_assignment: no conversion implemented for type kinds %zu -> %zu", 
            (usize) src_type.kind,
            (usize) dst_type.kind
        );
        exit(1);
    }
}
