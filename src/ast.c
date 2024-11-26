#include "ast.h"

#include <stdlib.h>
#include "slice.h"


void ast_debug(FILE *const stream, AstNode const *const node) {
    switch (node->kind) {
        case AstNodeKindIdentifier: {
            char *const name = charslice_as_cstr(node->identifier.name);
            fprintf(stream, "Identifier(name = %s)", name);
            free(name);
            break;
        }
        case AstNodeKindInteger: {
            fprintf(stream, "Integer(value = %d)", node->integer.value);
            break;
        }
        case AstNodeKindAssignment: {
            fprintf(stream, "Assignment(");
            fprintf(stream, "left = ");
            ast_debug(stream, node->addition.left);
            fprintf(stream, ", right = ");
            ast_debug(stream, node->addition.right);
            fprintf(stream, ")");
            break;
        }
        case AstNodeKindAddition: {
            fprintf(stream, "Addition(");
            fprintf(stream, "left = ");
            ast_debug(stream, node->addition.left);
            fprintf(stream, ", right = ");
            ast_debug(stream, node->addition.right);
            fprintf(stream, ")");
            break;
        }
        case AstNodeKindSubtraction: {
            fprintf(stream, "Subtraction(");
            fprintf(stream, "left = ");
            ast_debug(stream, node->addition.left);
            fprintf(stream, ", right = ");
            ast_debug(stream, node->addition.right);
            fprintf(stream, ")");
            break;
        }
        case AstNodeKindMultiplication: {
            fprintf(stream, "Multiplication(");
            fprintf(stream, "left = ");
            ast_debug(stream, node->addition.left);
            fprintf(stream, ", right = ");
            ast_debug(stream, node->addition.right);
            fprintf(stream, ")");
            break;
        }
        case AstNodeKindDivision: {
            fprintf(stream, "Division(");
            fprintf(stream, "left = ");
            ast_debug(stream, node->addition.left);
            fprintf(stream, ", right = ");
            ast_debug(stream, node->addition.right);
            fprintf(stream, ")");
            break;
        }
    }
}

