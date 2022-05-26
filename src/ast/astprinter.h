#ifndef _AST_ASTPRINTER_H_
#define _AST_ASTPRINTER_H_

#include <stdio.h>

#include "ast/ast.h"

void printAst(FILE* file, AstNode* ast);

void printAstIndented(FILE* file, AstNode* ast, int indent);

#endif
