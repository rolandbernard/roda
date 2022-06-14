
include config.mk

# == General
TARGETS := rodac
ALL_SWITCHES := llvm
# ==

# == Feature detection
LLVM_VERSION ?= $(shell llvm-config --version || true)
ifneq ($(LLVM_VERSION),)
LLVM ?= yes
endif
# ==

# == Enable features
ifeq ($(LLVM),yes)
SWITCHES += llvm
endif
# ==

# == Tools
CC   ?= gcc
CXX  ?= g++
ifeq ($(LLVM),yes)
LD   := $(CXX)
else
LD   := $(CC)
endif
LEX  ?= flex
YACC := bison
# ==

# == Flags
CFLAGS += -rdynamic
ifneq ($(GIT_HEAD),)
CFLAGS += -DGIT_HEAD="\"$(GIT_HEAD)\""
endif
ifneq ($(GIT_URL),)
CFLAGS += -DGIT_URL="\"$(GIT_URL)\""
endif
ifneq ($(LLVM_VERSION),)
CFLAGS += -DLLVM_VERSION="\"$(LLVM_VERSION)\""
endif
YFLAGS += -Wall

CFLAGS.llvm  += $(shell llvm-config --cflags || true)
LDFLAGS.llvm += $(shell llvm-config --ldflags || true)
LDLIBS.llvm  += $(shell llvm-config --libs --system-libs || true)
# ==

# == Parser generator files
LEX_SRC := $(SOURCE_DIR)/parser/lexer.l
LEX_C := $(SOURCE_DIR)/parser/lexer.yy.c
LEX_H := $(SOURCE_DIR)/parser/lexer.yy.h
YACC_SRC := $(SOURCE_DIR)/parser/parser.y
YACC_C := $(SOURCE_DIR)/parser/parser.tab.c
YACC_H := $(SOURCE_DIR)/parser/parser.tab.h

GEN_SOURCES += $(LEX_C) $(YACC_C)
TO_CLEAN += $(LEX_C) $(LEX_H) $(YACC_C) $(YACC_H)
# ==

include build.mk

# == Parser generator rules
$(LEX_C): $(LEX_SRC) $(YACC_C)
	@$(ECHO) "Building $@"
	$(LEX) $(LFLAGS) --outfile=$(LEX_C) --header-file=$(LEX_H) $<

$(YACC_C): $(YACC_SRC)
	@$(ECHO) "Building $@"
	$(YACC) $(YFLAGS) --output=$(YACC_C) --defines=$(YACC_H) $<
# ==

