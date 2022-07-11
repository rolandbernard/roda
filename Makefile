
include config.mk

# == General
TARGETS := rodac

ALL_TARGETS  := rodac tests
ALL_SWITCHES := llvm
# ==

# == Feature detection
LLVM_CONFIG  ?= llvm-config
LLVM_VERSION ?= $(shell $(LLVM_CONFIG) --version || true)
LLVM_MAJOR   ?= $(shell $(LLVM_CONFIG) --version | cut -d "." -f 1)
LLVM         ?= $(shell if [ $(LLVM_MAJOR) -ge 10 ] ; then echo yes ; else echo no ; fi)
# ==

# == Enable features
ifeq ($(LLVM),yes)
SWITCHES += llvm
endif
ifeq ($(TESTS),yes)
TARGETS += tests
endif
TARGETS := $(sort $(TARGETS))
SWITCHES := $(sort $(SWITCHES))

export SWITCHES TARGETS
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
# CFLAGS += -std=c11 -pedantic
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

CFLAGS.llvm  += $(shell $(LLVM_CONFIG) --cflags || true)
LDFLAGS.llvm += $(shell $(LLVM_CONFIG) --ldflags || true)
LDLIBS.llvm  += $(shell $(LLVM_CONFIG) --libs --system-libs || true)
LDLIBS.tests += -lpthread
# ==

# == Parser generator files
LEX_SRC := $(SOURCE_DIR)/parser/lexer.l
LEX_C := $(SOURCE_DIR)/parser/lexer.yy.c
LEX_H := $(SOURCE_DIR)/parser/lexer.yy.h
YACC_SRC := $(SOURCE_DIR)/parser/parser.y
YACC_C := $(SOURCE_DIR)/parser/parser.tab.c
YACC_H := $(SOURCE_DIR)/parser/parser.tab.h

GEN_SOURCES += $(LEX_C) $(YACC_C)
TO_CLEAN += $(LEX_C) $(LEX_H) $(YACC_C) $(YACC_H) $(BASE_DIR)/profile
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

# == Tetsing
.PHONY: test coverage

test: build
	@$(ECHO) "Starting build-in tests"
	$(BINARY_DIR)/tests
	@$(ECHO) "Starting tests with --debug"
	TEST_ARGS=--debug BINARY=$(BINARY_DIR)/rodac tested -j12 $(BASE_DIR)/tests
	@$(ECHO) "Starting tests with -O0"
	TEST_ARGS=-O0 BINARY=$(BINARY_DIR)/rodac tested -j12 $(BASE_DIR)/tests
	@$(ECHO) "Starting tests with -O2"
	TEST_ARGS=-O2 BINARY=$(BINARY_DIR)/rodac tested -j12 $(BASE_DIR)/tests
	@$(ECHO) "Finished tests"

coverage:
	$(RM) -r $(BASE_DIR)/profile/
	LLVM_PROFILE_FILE="profile/tests/%p.profraw" $(MAKE) BUILD=coverage TARGETS="rodac tests" test
	llvm-profdata merge $(BASE_DIR)/profile/tests/*.profraw -o $(BASE_DIR)/profile/combined.profdata
	llvm-cov show $(BUILD_DIR)/coverage/bin/rodac -instr-profile=$(BASE_DIR)/profile/combined.profdata -o $(BASE_DIR)/profile/report
	llvm-cov report $(BUILD_DIR)/coverage/bin/rodac -instr-profile=$(BASE_DIR)/profile/combined.profdata
# ==

