#include "cc/compile/call.h"

#include <stdlib.h>

#include "cc/compile/assembly.h"
#include "cc/compile/calling_convention.h"
#include "cc/compile/compiler.h"
#include "cc/compile/error.h"
#include "cc/compile/expression.h"
#include "cc/compile/function_table.h"
#include "cc/log.h"
#include "cc/function_signature.h"
#include "cc/type.h"

struct CompileResult compile_remaining_arguments(
    struct Compiler *compiler,
    struct AstCall const *const ast,
    struct ArgumentLocationContext *const argument_location_context,
    struct FunctionSignature const *const signature,
    usize argument_index,
    usize *const temporary_stack_usage
) {
    if (argument_index >= ast->argument_count) {
        return compile_ok();
    }

    struct Type const parameter_type = signature->parameters[argument_index].type;
    struct Operand const argument_location 
        = locate_next_argument(argument_location_context, &parameter_type);

    // compile argument

    struct ExpressionValue argument_value;
    compile_expression(
        &argument_value, 
        compiler,
        &ast->arguments[argument_index]
    );

    // type checking

    if (!type_can_coerce(&parameter_type, &argument_value.type)) {
        return compile_error((struct CompileError) {
            .kind = CompileErrorIncompatibleTypes,
            .position = ast->arguments[argument_index].position,
            .variant.incompatible_types = {
                .first = parameter_type,
                .second = argument_value.type,
            },
        });
    }

    // if argument is stored in register, move to temporary stack location

    if (argument_value.operand.kind == OperandRegister) {
        usize const size_bytes = type_size_bytes(&argument_value.type);
        struct Operand const operand_dst 
            = compiler_allocate_temporary_stack_space(compiler, size_bytes);

        emit_move(
            &compiler->writer_function_body, 
            operand_dst, 
            argument_value.operand, 
            integer_operand_width(parameter_type.variant.integer_type.size), 
            integer_operand_width(argument_value.type.variant.integer_type.size), 
            RegisterA
        );

        argument_value.operand = operand_dst;
        *temporary_stack_usage += size_bytes;
    }

    // compile remaining arguments

    struct CompileResult result = compile_remaining_arguments(
        compiler, 
        ast,
        argument_location_context, 
        signature, 
        argument_index + 1u,
        temporary_stack_usage
    );

    if (!result.ok) {
        return result;
    }

    // move argument to passed location (reverse order)

    if (argument_location.kind == OperandRegister) {
        // move

        emit_assignment(
            &compiler->writer_function_body, 
            argument_location, 
            argument_value.operand,
            parameter_type,
            argument_value.type
        );
    } else if (argument_location.kind == OperandMemory) {
        // push

        if (type_eq(&parameter_type, &argument_value.type)) {
            emit_instruction_single_operand(
                &compiler->writer_function_body, 
                InstructionPush,
                integer_operand_width(parameter_type.variant.integer_type.size), 
                argument_value.operand
            );
        } else {
            // perform type conversion
            // NB: assumes type fits in register (should be true for C for any coerced type)
            emit_assignment(
                &compiler->writer_function_body, 
                operand_register(RegisterA), 
                argument_value.operand, 
                parameter_type, 
                argument_value.type
            );
            emit_instruction_single_operand(
                &compiler->writer_function_body, 
                InstructionPush,
                integer_operand_width(parameter_type.variant.integer_type.size), 
                operand_register(RegisterA)
            );
        }
    } else {
        log_error("bad argument location");
        exit(1);
    }

    return compile_ok();
}

struct CompileResult compile_call(
    struct ExpressionValue *value_out, 
    struct Compiler *compiler, 
    struct AstCall const *ast
) {
    // analyze types
        
    struct FunctionDescription function_desc;
    bool exists = function_table_get(
        compiler->function_table, 
        ast->callee.name, 
        &function_desc
    );

    if (!exists) {
        return compile_error((struct CompileError) {
            .kind = CompileErrorUndeclaredIdentifier,
            .position = ast->callee.position,
            .variant.undeclared_identifier = {
                .name = ast->callee.name,
            },
        });
    }

    if (ast->argument_count != function_desc.signature.parameter_count) {
        return compile_error((struct CompileError) {
            .kind = CompileErrorIncorrectArgumentCount,
            .position = ast->callee.position,
            .variant.incorrect_argument_count = {
                .function_name = function_desc.name,
                .expected = function_desc.signature.parameter_count,
                .got = ast->argument_count
            },
        });
    }

    // compile arguments

    usize temporary_stack_usage = 0u;

    struct ArgumentLocationContext argument_location_context;
    argument_location_context_init(&argument_location_context);

    compile_remaining_arguments(
        compiler, 
        ast, 
        &argument_location_context, 
        &function_desc.signature, 
        0u, 
        &temporary_stack_usage
    );

    // emit call 
    
    emit_instruction_single_operand(
        &compiler->writer_function_body, 
        InstructionCall, 
        QWord, 
        operand_label(function_desc.name)
    );

    // cleanup stack

    if (argument_location_context.stack_displacement > 0u) {
        emit_instruction_dst_src(
            &compiler->writer_function_body, 
            InstructionAdd, 
            QWord, 
            QWord, 
            operand_register(RegisterSP),
            operand_immediate(argument_location_context.stack_displacement)
        );
    }

    compiler_free_temporary_stack_space(compiler, temporary_stack_usage);

    value_out->operand = operand_register(RegisterA);
    value_out->type = function_desc.signature.return_type;

    return compile_ok();
}

