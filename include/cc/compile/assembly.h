#pragma once

#include "cc/common.h"
#include "cc/type.h"
#include "cc/writer.h"

enum Instruction {
    // 0 operands
    InstructionLeave,
    InstructionRet,
    InstructionCdq,
    InstructionCdqe,
    // 1 operand
    InstructionPush,
    InstructionPop,
    InstructionCall,
    InstructionIDiv,
    // 2 operands
    InstructionMov,
    InstructionMovSx,
    InstructionMovZx,
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
            u64 value;
        } immediate;
        struct {
            struct CharSlice name;
        } label;
        struct {
            enum IntRegister reg;
        } int_register;
        struct {
            enum IntRegister base_reg;
            i64 displacement;
        } memory;
        struct {
            enum IntRegister base_reg;
            enum IntRegister index_reg;
            i64 displacement;
            i64 index_scale;
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

enum OperandWidth integer_operand_width(enum IntegerSize size);

char const *format_operand_width(enum OperandWidth operand_width);
char const *format_register(enum IntRegister reg, enum OperandWidth operand_width);
char const *format_instruction(enum Instruction instruction);
usize instruction_expected_operand_count(enum Instruction instruction);

struct Operand operand_immediate(u64 value);
struct Operand operand_label(struct CharSlice name);
struct Operand operand_register(enum IntRegister reg);
struct Operand operand_memory(enum IntRegister base_reg, i64 offset);
struct Operand operand_memory_indexed(
    enum IntRegister base_reg, 
    enum IntRegister index_reg, 
    i64 displacement, 
    i64 index_scale
);
struct Operand operand_stack(usize stack_offset);

// only changes memory or memory indexed operands
// returns the operand corresponding to the original operand displaced by `amount_bytes`
struct Operand operand_displace(struct Operand operand, i64 amount_bytes);

void emit_operand(struct Writer *assembly_writer, struct Operand operand, enum OperandWidth width);
void emit_instruction(struct Writer *assembly_writer, enum Instruction instruction);
void emit_instruction_single_operand(
    struct Writer *assembly_writer, 
    enum Instruction instruction, 
    enum OperandWidth operand_width,
    struct Operand operand
);
void emit_instruction_dst_src(
    struct Writer *assembly_writer, 
    enum Instruction instruction, 
    enum OperandWidth dst_width,
    enum OperandWidth src_width,
    struct Operand dst,
    struct Operand src
);

void emit_label(struct Writer *assembly_writer, struct CharSlice label);
void emit_function_prologue(struct Writer *assembly_writer, usize stack_usage);
void emit_function_exit(struct Writer *assembly_writer);

// emit mov instructions as necessary to move src to dst 
// may use `intermediate_register` as necessary
// (0-2 instructions)
void emit_move(
    struct Writer *assembly_writer, 
    struct Operand dst, 
    struct Operand src, 
    enum OperandWidth dst_operand_width,
    enum OperandWidth src_operand_width,
    enum IntRegister intermediate_register
);

// emit mov instructions as necessary `size_bytes` from src to dst 
// dst must not be a register if amount_bytes > 8
// may use `intermediate_register` as necessary
void emit_move_bytes(
    struct Writer *assembly_writer, 
    struct Operand dst, 
    struct Operand src, 
    usize size_bytes,
    enum IntRegister intermediate_register
);

// emit mov instructions as necessary to move `n` bytes from src to dst 
// may use `intermediate_register` as necessary

// move src to dst with type conversions
void emit_assignment(
    struct Writer *assembly_writer,
    struct Operand dst,
    struct Operand src,
    struct Type dst_type,
    struct Type src_type
);

