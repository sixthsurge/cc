#pragma once

#include "cc/common.h"
#include "cc/slice.h"
#include "cc/token.h"

struct Writer;

enum AstUnaryOpKind {
    AstUnaryOpNegation,
};

enum AstBinaryOpKind {
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
    AstExpressionAssignment,
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

struct AstNodePosition {
    struct TokenPosition position_start;
    struct TokenPosition position_end;
};

struct AstType {
    struct AstNodePosition position;
    enum AstTypeKind kind;
};

struct AstIdentifier {
    struct AstNodePosition position;
    struct CharSlice name;
};

struct AstAssignee {
    struct AstNodePosition position;
    struct AstIdentifier identifier;
};

struct AstAssignment {
    struct AstNodePosition position;
    struct AstAssignee assignee;
    struct AstExpression const *assigned_expression;
};

struct AstConstant {
    struct AstNodePosition position;
    i32 value;
};

struct AstCall {
    struct AstNodePosition position;
    struct AstIdentifier callee;
    struct AstExpression *arguments;
    usize argument_count;
}; 

struct AstUnaryOp {
    enum AstUnaryOpKind kind;
    struct AstNodePosition position;
    struct AstExpression const *expression;
};

struct AstBinaryOp {
    enum AstBinaryOpKind kind;
    struct AstNodePosition position;
    struct AstExpression const *left;
    struct AstExpression const *right;
};

struct AstExpression {
    enum AstExpressionKind kind;
    struct AstNodePosition position;

    union {
        struct AstIdentifier identifier;
        struct AstConstant constant;
        struct AstAssignment assignment;
        struct AstCall call;
        struct AstUnaryOp unary_op;
        struct AstBinaryOp binary_op;
    } variant;
};

struct AstVariableDeclaration {
    struct AstNodePosition position;
    struct AstIdentifier identifier;
    struct AstType type;
    struct AstExpression assigned_expression;
    bool has_assigned_expression;
};

struct AstReturn {
    bool has_returned_expression;
    struct AstNodePosition position;
    struct AstExpression returned_expression;
};

struct AstStatement {
    enum AstStatementKind kind;
    struct AstNodePosition position;

    union {
        struct AstExpression expression;
        struct AstVariableDeclaration variable_declaration;
        struct AstReturn return_statement;
    } variant;
};

struct AstBlock {
    struct AstNodePosition position;
    struct AstStatement const *statements;
    usize statement_count;
};

struct AstFunctionParameter {
    struct AstNodePosition position;
    struct AstIdentifier identifier;
    struct AstType type;
    bool has_identifier;
};

struct AstFunctionSignature {
    struct AstNodePosition position;
    struct AstIdentifier identifier;
    struct AstType return_type;
    struct AstFunctionParameter *parameters;
    usize parameter_count;
};

struct AstFunctionDefinition {
    struct AstNodePosition position;
    struct AstFunctionSignature signature;
    struct AstBlock body;
};

struct AstTopLevelItem {
    enum AstTopLevelItemKind kind;
    struct AstNodePosition position;

    union {
        struct AstFunctionDefinition function_definition;
    } variant;
};

struct AstRoot {
    struct AstTopLevelItem const *items;
    usize item_count;
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
void ast_debug_assignment(struct Writer *writer, struct AstAssignment const *self);
void ast_debug_assignee(struct Writer *writer, struct AstAssignee const *self);
void ast_debug_unary_op(struct Writer *writer, struct AstUnaryOp const *self);
void ast_debug_binary_op(struct Writer *writer, struct AstBinaryOp const *self);
void ast_debug_call(struct Writer *writer, struct AstCall const *self);
void ast_debug_constant(struct Writer *writer, struct AstConstant const *self);
void ast_debug_identifier(struct Writer *writer, struct AstIdentifier const *self);
void ast_debug_type(struct Writer *writer, struct AstType const *self);

