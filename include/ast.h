#pragma once

#include <stdio.h>
#include "slice.h"
#include "type.h"
#include "vec.h"

struct AstNode;
struct Writer;

typedef enum AstNodeKind {
    AstNodeBlock,

    // Statements 
    AstNodeDeclaration,
    AstNodeReturn,

    // Expressions
    AstNodeAssignment,
    AstNodeAddition,
    AstNodeSubtraction,
    AstNodeMultiplication,
    AstNodeDivision,

    // Terminals
    AstNodeIdentifier,
    AstNodeInteger,
} AstNodeKind;

typedef struct AstBlock {
    PtrVec nodes;
} AstBlock;

typedef struct AstDeclaration {
    CharSlice name;
    struct AstNode const *expression;
} AstDeclaration;

typedef struct AstReturn {
    struct AstNode const *expression;
} AstReturn;

typedef struct AstAssignment {
    struct AstNode const *left;
    struct AstNode const *right;
} AstAssignment;

typedef struct AstAddition {
    struct AstNode const *left;
    struct AstNode const *right;
} AstAddition;

typedef struct AstSubtraction {
    struct AstNode const *left;
    struct AstNode const *right;
} AstSubtraction;

typedef struct AstMultiplication {
    struct AstNode const *left;
    struct AstNode const *right;
} AstMultiplication;

typedef struct AstDivision {
    struct AstNode const *left;
    struct AstNode const *right;
} AstDivision;

typedef struct AstIdentifier {
    CharSlice name;
} AstIdentifier;

typedef struct AstInteger {
    i32 value;
} AstInteger;

typedef struct AstNode {
    AstNodeKind kind;

    union {
        AstBlock          block;
        AstDeclaration    declaration;
        AstReturn         return_statement;
        AstAssignment     assignment;
        AstAddition       addition;
        AstSubtraction    subtraction;
        AstMultiplication multiplication;
        AstDivision       division;
        AstIdentifier     identifier;
        AstInteger        integer;
    };
} AstNode;

void ast_debug(struct Writer *writer, AstNode const *node);
