#ifndef _AST_ASTPRINTER_H_
#define _AST_ASTPRINTER_H_

#include <stdio.h>

#include "compiler/context.h"
#include "ast/ast.h"

void printAst(FILE* file, CompilerContext* context, AstNode* ast);

#endif
