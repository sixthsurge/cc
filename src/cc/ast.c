#include "cc/ast.h"

#include "cc/writer.h"

void ast_debug_root(struct Writer *writer, struct AstRoot const *self) {
    writer_write(writer, "Root(items = [");
    for (usize i = 0; i < self->item_count; ++i) {
        ast_debug_top_level_item(writer, &self->items[i]);

        if (i < self->item_count - 1) {
            writer_write(writer, ", ");
        }
    }
    writer_write(writer, "])");
}

void ast_debug_top_level_item(struct Writer *writer, struct AstTopLevelItem const *self) {
    switch (self->kind) {
        case AstTopLevelItemFunctionDefinition: {
            ast_debug_function_definition(writer, &self->function_definition);
            break;
        }       
    }
}

void ast_debug_function_definition(struct Writer *writer, struct AstFunctionDefinition const *self) {
    writer_write(writer, "FunctionDefinition(");

    writer_write(writer, "signature = ");
    ast_debug_function_signature(writer, &self->signature);
    
    writer_write(writer, ", body = ");
    ast_debug_block(writer, &self->body);

    writer_write(writer, ")");
}

void ast_debug_function_signature(struct Writer *writer, struct AstFunctionSignature const *self) {
    writer_write(writer, "FunctionSignature(");

    writer_write(writer, "identifier = ");
    ast_debug_identifier(writer, &self->identifier);

    writer_write(writer, ", return_type = ");
    ast_debug_type(writer, &self->return_type);

    writer_write(writer, ", arguments = [");
    for (usize i = 0; i < self->parameter_count; ++i) {
        ast_debug_function_parameter(writer, &self->parameters[i]);

        if (i < self->parameter_count - 1) {
            writer_write(writer, ", ");
        }
    }

    writer_write(writer, "])");
}

void ast_debug_function_parameter(struct Writer *writer, struct AstFunctionParameter const *self) {
    writer_write(writer, "FunctionParameter(");

    writer_write(writer, "identifier = ");
    if (self->has_identifier) {
        ast_debug_identifier(writer, &self->identifier);
    } else {
        writer_write(writer, "None");
    }

    writer_write(writer, ", type = ");
    ast_debug_type(writer, &self->type);

    writer_write(writer, "])");
}

void ast_debug_block(struct Writer *writer, struct AstBlock const *self) {
    writer_write(writer, "Block(statements = [");
    for (usize i = 0; i < self->statement_count; ++i) {
        ast_debug_statement(writer, &self->statements[i]);

        if (i < self->statement_count - 1) {
            writer_write(writer, ", ");
        }
    }
    writer_write(writer, "])");
}

void ast_debug_statement(struct Writer *writer, struct AstStatement const *self) {
    switch (self->kind) {
        case AstStatementExpression: {
            ast_debug_expression(writer, &self->expression);
            break;
        }
        case AstStatementVariableDeclaration: {
            ast_debug_variable_declaration(writer, &self->variable_declaration);
            break;
        }
        case AstStatementReturn: {
            ast_debug_return(writer, &self->return_statement);
            break;
        }
    }
}

void ast_debug_variable_declaration(struct Writer *writer, struct AstVariableDeclaration const *self) {
    writer_write(writer, "VariableDeclaration(");

    writer_write(writer, "identifier = ");
    ast_debug_identifier(writer, &self->identifier);

    writer_write(writer, ", type = ");
    ast_debug_type(writer, &self->type);

    writer_write(writer, ", assigned_expression = ");
    if (self->has_assigned_expression) {
        ast_debug_expression(writer, &self->assigned_expression);
    } else {
        writer_write(writer, "None");
    }

    writer_write(writer, ")");
}

void ast_debug_return(struct Writer *writer, struct AstReturn const *self) {
    writer_write(writer, "Return(");
    writer_write(writer, "returned_expression = ");
    if (self->has_returned_expression) {
        ast_debug_expression(writer, &self->returned_expression);
    } else {
        writer_write(writer, "None");
    }
    writer_write(writer, ")");
}

void ast_debug_expression(struct Writer *writer, struct AstExpression const *self) {
    switch (self->kind) {
        case AstExpressionIdentifier: {
            ast_debug_identifier(writer, &self->identifier);
            break;
        }
        case AstExpressionConstant: {
            ast_debug_constant(writer, &self->constant);
            break;
        }
        case AstExpressionAssignment: {
            ast_debug_assignment(writer, &self->assignment);
            break;
        }
        case AstExpressionCall: {
            ast_debug_call(writer, &self->call);
            break;
        }
        case AstExpressionUnaryOp: {
            ast_debug_unary_op(writer, &self->unary_op);
            break;
        }
        case AstExpressionBinaryOp: {
            ast_debug_binary_op(writer, &self->binary_op);
            break;
        }
    }
}

void ast_debug_assignment(struct Writer *writer, struct AstAssignment const *self) {
    writer_write(writer, "Assignment(");
    writer_write(writer, "assignee = ");
    ast_debug_assignee(writer, &self->assignee);
    writer_write(writer, ", ");
    writer_write(writer, "assigned_expression = ");
    ast_debug_expression(writer, self->assigned_expression);
    writer_write(writer, ")");
}

void ast_debug_assignee(struct Writer *writer, struct AstAssignee const *self) {
    ast_debug_identifier(writer, &self->identifier);
}

void ast_debug_unary_op(struct Writer *writer, struct AstUnaryOp const *self) {
    writer_write(writer, "UnaryOp(");

    writer_write(writer, "kind = ");
    switch (self->kind) {
        case AstUnaryOpNegation: { 
            writer_write(writer, "Negation");
            break;
        }
    }

    writer_write(writer, ", expression = ");
    ast_debug_expression(writer, self->expression);
    
    writer_write(writer, ")");
}

void ast_debug_binary_op(struct Writer *writer, struct AstBinaryOp const *self) {
    writer_write(writer, "BinaryOp(");

    writer_write(writer, "kind = ");
    switch (self->kind) {
        case AstBinaryOpAddition: { 
            writer_write(writer, "Addition");
            break;
        }
        case AstBinaryOpSubtraction: { 
            writer_write(writer, "Subtraction");
            break;
        }
        case AstBinaryOpMultiplication: { 
            writer_write(writer, "Multiplication");
            break;
        }
        case AstBinaryOpDivision: { 
            writer_write(writer, "Division");
            break;
        }
    }

    writer_write(writer, ", left = ");
    ast_debug_expression(writer, self->left);

    writer_write(writer, ", right = ");
    ast_debug_expression(writer, self->right);
    
    writer_write(writer, ")");
}

void ast_debug_call(struct Writer *writer, struct AstCall const *self) {
    writer_write(writer, "Call(");

    writer_write(writer, "callee = ");
    ast_debug_expression(writer, self->callee);

    writer_write(writer, "arguments = [");
    for (usize i = 0; i < self->argument_count; ++i) {
        ast_debug_expression(writer, &self->arguments[i]);

        if (i < self->argument_count - 1) {
            writer_write(writer, ", ");
        }
    }
    writer_write(writer, "])");
}

void ast_debug_constant(struct Writer *writer, struct AstConstant const *self) {
    writer_writef(writer, "Constant(value = %d)", self->value);
}

void ast_debug_identifier(struct Writer *writer, struct AstIdentifier const *self) {
    writer_write(writer, "Identifier(name = ");
    writer_write_charslice(writer, self->name);
    writer_write(writer, ")");
}

void ast_debug_type(struct Writer *writer, struct AstType const *self) {
    writer_write(writer, "int");
}
