#pragma once

#include "cc/common.h"
#include "cc/type.h"
#include "cc/writer.h"

enum Instruction {
    // 0 operands
    InstructionLeave,
    InstructionRet,
    InstructionCdq,
    // 1 operand
    InstructionPush,
    InstructionPop,
    InstructionCall,
    InstructionIDiv,
    // 2 operands
    InstructionMov,
    InstructionAdd,
    InstructionSub,
    InstructionIMul,
    InstructionCount,
};

enum IntRegister {
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

enum OperandKind {
    OperandImmediate,
    OperandLabel,
    OperandRegister,
    OperandMemory,
    OperandMemoryIndexed,
};

struct Operand {
    enum OperandKind kind;

    union {
        struct {
            i32 value;
        } immediate;
        struct {
            struct CharSlice name;
        } label;
        struct {
            enum IntRegister reg;
        } int_register;
        struct {
            enum IntRegister base_reg;
            i32 offset;
        } memory;
        struct {
            enum IntRegister base_reg;
            enum IntRegister index_reg;
            i32 offset;
            i32 index_scale;
        } memory_indexed;
    } variant;
};

enum OperandWidth {
    Byte,
    Word,
    DWord,
    QWord,
    OperandWidthCount
};

char const *format_operand_width(enum OperandWidth operand_width);
char const *format_register(enum IntRegister reg, enum OperandWidth operand_width);
char const *format_instruction(enum Instruction instruction);
usize instruction_expected_operand_count(enum Instruction instruction);

struct Operand operand_immediate(i32 value);
struct Operand operand_label(struct CharSlice name);
struct Operand operand_register(enum IntRegister reg);
struct Operand operand_memory(enum IntRegister base_reg, i32 offset);
struct Operand operand_memory_indexed(
    enum IntRegister base_reg, 
    enum IntRegister index_reg, 
    i32 offset, 
    i32 index_scale
);
struct Operand operand_stack(usize stack_offset);

void emit_operand(struct Writer *assembly_writer, struct Operand operand, enum OperandWidth width);
void emit_instruction(
    struct Writer *assembly_writer, 
    enum Instruction instruction,
    enum OperandWidth operand_width,
    usize operand_count, 
    ...
);

void emit_label(struct Writer *assembly_writer, struct CharSlice label);
void emit_function_prologue(struct Writer *assembly_writer, usize stack_usage);
void emit_function_exit(struct Writer *assembly_writer);

// emit mov instructions as necessary to move src to dst 
// may use `intermediate_register` as necessary
// (0-2 instructions)
void emit_moves(
    struct Writer *assembly_writer, 
    struct Operand dst, 
    struct Operand src, 
    enum OperandWidth operand_width,
    enum IntRegister intermediate_register
);

void emit_assign_variable(
    struct Writer *assembly_writer,
    usize assignee_stack_offset,
    struct Type assignee_type,
    struct Operand assigned_location,
    struct Type assigned_type
);

