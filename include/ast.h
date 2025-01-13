#pragma once

#include "common.h"
#include "slice.h"

struct Writer;

enum AstUnaryOpKind {
    AstUnaryOpNegation,
};

enum AstBinaryOpKind {
    AstBinaryOpAssignment,
    AstBinaryOpAddition,
    AstBinaryOpSubtraction,
    AstBinaryOpMultiplication,
    AstBinaryOpDivision,
};

enum AstTypeKind {
    AstTypeIntS32,
};

enum AstExpressionKind {
    AstExpressionIdentifier,
    AstExpressionConstant,
    AstExpressionCall,
    AstExpressionUnaryOp,
    AstExpressionBinaryOp,
};

enum AstStatementKind {
    AstStatementExpression,
    AstStatementVariableDeclaration,
    AstStatementReturn,
};

enum AstTopLevelItemKind {
    AstTopLevelItemFunctionDefinition,
};

struct AstExpression;
struct AstStatement;

struct AstType {
    enum AstTypeKind kind;
};

struct AstIdentifier {
    struct CharSlice name;
};

struct AstConstant {
    i32 value;
};

struct AstCall {
    struct AstExpression const *callee;
    struct AstExpression const *arguments;
    usize argument_count;
}; 

struct AstUnaryOp {
    enum AstUnaryOpKind kind;
    struct AstExpression const *expression;
};

struct AstBinaryOp {
    enum AstBinaryOpKind kind;
    struct AstExpression const *left;
    struct AstExpression const *right;
};

struct AstExpression {
    enum AstExpressionKind kind;

    union {
        struct AstIdentifier identifier;
        struct AstConstant constant;
        struct AstCall call;
        struct AstUnaryOp unary_op;
        struct AstBinaryOp binary_op;
    };
};

struct AstVariableDeclaration {
    struct AstIdentifier identifier;
    struct AstType type;
    struct AstExpression assigned_expression;
    bool has_assigned_expression;
};

struct AstReturn {
    struct AstExpression expression;
};

struct AstStatement {
    enum AstStatementKind kind;

    union {
        struct AstExpression expression;
        struct AstVariableDeclaration variable_declaration;
        struct AstReturn return_statement;
    };
};

struct AstBlock {
    struct AstStatement const *statements;
    usize statement_count;
};

struct AstFunctionParameter {
    struct AstIdentifier identifier;
    struct AstType type;
};

struct AstFunctionSignature {
    struct AstIdentifier identifier;
    struct AstType return_type;
    struct AstFunctionParameter *parameters;
    usize parameter_count;
};

struct AstFunctionDefinition {
    struct AstFunctionSignature signature;
    struct AstBlock body;
};

struct AstTopLevelItem {
    enum AstTopLevelItemKind kind;

    union {
        struct AstFunctionDefinition function_definition;
    };
};

struct AstRoot {
    //struct AstTopLevelItem const *items;
    //usize item_count;
    struct AstBlock block;
};

void ast_debug_root(struct Writer *writer, struct AstRoot const *self);
void ast_debug_top_level_item(struct Writer *writer, struct AstTopLevelItem const *self);
void ast_debug_function_definition(struct Writer *writer, struct AstFunctionDefinition const *self);
void ast_debug_function_signature(struct Writer *writer, struct AstFunctionSignature const *self);
void ast_debug_function_parameter(struct Writer *writer, struct AstFunctionParameter const *parameter);
void ast_debug_block(struct Writer *writer, struct AstBlock const *self);
void ast_debug_statement(struct Writer *writer, struct AstStatement const *self);
void ast_debug_variable_declaration(struct Writer *writer, struct AstVariableDeclaration const *self);
void ast_debug_return(struct Writer *writer, struct AstReturn const *self);
void ast_debug_expression(struct Writer *writer, struct AstExpression const *self);
void ast_debug_unary_op(struct Writer *writer, struct AstUnaryOp const *self);
void ast_debug_binary_op(struct Writer *writer, struct AstBinaryOp const *self);
void ast_debug_call(struct Writer *writer, struct AstCall const *self);
void ast_debug_constant(struct Writer *writer, struct AstConstant const *self);
void ast_debug_identifier(struct Writer *writer, struct AstIdentifier const *self);
void ast_debug_type(struct Writer *writer, struct AstType const *self);

