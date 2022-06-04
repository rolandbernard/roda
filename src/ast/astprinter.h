#ifndef _AST_ASTPRINTER_H_
#define _AST_ASTPRINTER_H_

#include <stdio.h>

#include "compiler/context.h"
#include "ast/ast.h"

const char* getAstPrintName(AstNodeKind kind);

void printAst(FILE* file, AstNode* ast);

#endif
