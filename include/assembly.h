#pragma once

#include "common.h"
#include "writer.h"

enum Instruction {
    // 0 operands
    InstructionLeave,
    InstructionRet,
    // 1 operand
    InstructionPush,
    InstructionPop,
    // 2 operands
    InstructionMov,
    InstructionAdd,
    InstructionSub,
    InstructionCount,
};

enum Register {
    RegisterA,
    RegisterB,
    RegisterC,
    RegisterD,
    RegisterSI,
    RegisterDI,
    RegisterSP,
    RegisterBP,
    Register8,
    Register9,
    Register10,
    Register11,
    Register12,
    Register13,
    Register14,
    Register15,
    RegisterCount,
};

enum OperandWidth {
    Byte,
    Word,
    DWord,
    QWord,
    OperandWidthCount
};

enum OperandKind {
    OperandImmediate,
    OperandRegister,
    OperandMemory,
    OperandMemoryIndexed,
};

struct Operand {
    enum OperandKind kind;
    enum OperandWidth width;

    union {
        struct {
            i32 value;
        } immediate;
        struct {
            enum Register reg;
        } reg;
        struct {
            enum Register base_reg;
            i32 offset;
        } memory;
        struct {
            enum Register base_reg;
            enum Register index_reg;
            i32 offset;
            i32 index_scale;
        } memory_indexed;
    };
};

char const *format_operand_width(enum OperandWidth operand_width);
char const *format_register(enum Register reg, enum OperandWidth operand_width);
char const *format_instruction(enum Instruction instruction);
usize instruction_expected_operand_count(enum Instruction instruction);

struct Operand operand_immediate(enum OperandWidth width, i32 value);
struct Operand operand_register(enum OperandWidth width, enum Register reg);
struct Operand operand_memory(enum OperandWidth width, enum Register base_reg, i32 offset);
struct Operand operand_memory_indexed(
    enum OperandWidth width,
    enum Register base_reg, 
    enum Register index_reg, 
    i32 offset, 
    i32 index_scale
);
struct Operand operand_stack(enum OperandWidth width, usize stack_offset);

void write_operand(struct Writer *assembly_writer, struct Operand operand);
void write_instruction(
    struct Writer *assembly_writer, 
    enum Instruction instruction,
    usize operand_count, 
    ...
);

