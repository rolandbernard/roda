#ifndef _PARSER_WRAPPER_H_
#define _PARSER_WRAPPER_H_

#include "ast/ast.h"
#include "errors/msgcontext.h"

AstNode* parseStdin(MessageContext* context);

#endif
