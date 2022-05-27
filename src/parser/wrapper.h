#ifndef _PARSER_WRAPPER_H_
#define _PARSER_WRAPPER_H_

#include "ast/ast.h"
#include "errors/msgcontext.h"

AstNode* parseFile(File* file, MessageContext* context);

AstNode* parseStdin(MessageContext* context);

#endif
