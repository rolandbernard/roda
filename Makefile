
include config.mk

# == General
TARGETS := rodac
ALL_SWITCHES := llvm
# ==

# == Feature detection
LLVM ?= yes
# ==

# == Enable features
ifeq ($(LLVM),yes)
SWITCHES += llvm
endif
# ==

# == Tools
CC ?= gcc
LD := $(CC)
# ==

# == Flags
CCFLAGS.llvm += $(shell llvm-config --cflags || true)
LDFLAGS.llvm += $(shell llvm-config --libs --ldflags --system-libs || true)
# ==

# == Parser generator files
LEX_SRC := $(SOURCE_DIR)/parser/lexer.l
LEX_C := $(SOURCE_DIR)/parser/lexer.yy.c
LEX_H := $(SOURCE_DIR)/parser/lexer.yy.h
YACC_SRC := $(SOURCE_DIR)/parser/parser.y
YACC_C := $(SOURCE_DIR)/parser/parser.tab.c
YACC_H := $(SOURCE_DIR)/parser/parser.tab.h

ALL_SOURCES += $(LEX_C) $(YACC_C)
TO_CLEAN += $(LEX_C) $(LEX_H) $(YACC_C) $(YACC_H)
# ==

include build.mk

# == Parser generator rules
$(LEX_C): $(LEX_SRC) $(YACC_C)
	flex --outfile=$(LEX_C) --header-file=$(LEX_H) $<

$(YACC_C): $(YACC_SRC)
	bison -Wall --output=$(YACC_C) --defines=$(YACC_H) $<
# ==

