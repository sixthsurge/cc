#include "ast.h"

#include <stdlib.h>
#include "slice.h"
#include "writer.h"

void ast_debug(struct Writer *writer, AstNode const *const node) {
    switch (node->kind) {
        case AstNodeBlock: {
            writer_writef(writer, "Block(");
            AstBlock const block = node->block;

            for (usize i = 0; i < block.nodes.len; i += 1) {
                ast_debug(writer, (AstNode *) *ptrvec_at(&block.nodes, i));

                if (i + 1 < block.nodes.len) {
                    writer_writef(writer, ", ");
                }
            }

            writer_writef(writer, ")");
            break;
        }
        case AstNodeDeclaration: {
            writer_writef(writer, "Declaration(");
            writer_writef(writer, "name = %s, ", charslice_as_cstr(node->declaration.name));
            writer_writef(writer, "expression = ");
            ast_debug(writer, node->declaration.expression);
            writer_writef(writer, ")");
            break;
        }
        case AstNodeReturn: {
            writer_writef(writer, "Return(");
            ast_debug(writer, node->return_statement.expression);
            writer_writef(writer, ")");
            break;
        }
        case AstNodeIdentifier: {
            char *const name = charslice_as_cstr(node->identifier.name);
            writer_writef(writer, "Identifier(%s)", name);
            free(name);
            break;
        }
        case AstNodeInteger: {
            writer_writef(writer, "Integer(%d)", node->integer.value);
            break;
        }
        case AstNodeAssignment: {
            writer_writef(writer, "Assignment(");
            writer_writef(writer, "left = ");
            ast_debug(writer, node->addition.left);
            writer_writef(writer, ", right = ");
            ast_debug(writer, node->addition.right);
            writer_writef(writer, ")");
            break;
        }
        case AstNodeAddition: {
            writer_writef(writer, "Addition(");
            writer_writef(writer, "left = ");
            ast_debug(writer, node->addition.left);
            writer_writef(writer, ", right = ");
            ast_debug(writer, node->addition.right);
            writer_writef(writer, ")");
            break;
        }
        case AstNodeSubtraction: {
            writer_writef(writer, "Subtraction(");
            writer_writef(writer, "left = ");
            ast_debug(writer, node->addition.left);
            writer_writef(writer, ", right = ");
            ast_debug(writer, node->addition.right);
            writer_writef(writer, ")");
            break;
        }
        case AstNodeMultiplication: {
            writer_writef(writer, "Multiplication(");
            writer_writef(writer, "left = ");
            ast_debug(writer, node->addition.left);
            writer_writef(writer, ", right = ");
            ast_debug(writer, node->addition.right);
            writer_writef(writer, ")");
            break;
        }
        case AstNodeDivision: {
            writer_writef(writer, "Division(");
            writer_writef(writer, "left = ");
            ast_debug(writer, node->addition.left);
            writer_writef(writer, ", right = ");
            ast_debug(writer, node->addition.right);
            writer_writef(writer, ")");
            break;
        }
    }
}

