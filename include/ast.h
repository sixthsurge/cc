#pragma once

#include <stdio.h>
#include "slice.h"

struct AstNode;

typedef enum AstNodeKind {
    // Terminals
    AstNodeKindIdentifier,
    AstNodeKindInteger,

    // Operators
    AstNodeKindAssignment,
    AstNodeKindAddition,
    AstNodeKindSubtraction,
    AstNodeKindMultiplication,
    AstNodeKindDivision,

    // Statements
    AstNodeKindDeclaration,
} AstNodeKind;

typedef struct AstIdentifier {
    CharSlice name;
} AstIdentifier;

typedef struct AstInteger {
    i32 value;
} AstInteger;

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

typedef struct AstNode {
    AstNodeKind kind;

    union {
        AstIdentifier     identifier;
        AstInteger        integer;
        AstAssignment     assignment;
        AstAddition       addition;
        AstSubtraction    subtraction;
        AstMultiplication multiplication;
        AstDivision       division;
    };
} AstNode;

void ast_debug(FILE *stream, AstNode const *node);
