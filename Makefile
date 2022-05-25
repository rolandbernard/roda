
include config.mk

# == Config
TARGETS := compiler
# ==

# == Tools
CC ?= gcc
LD := $(CC)
# ==

# == Parser generator
ALL_SOURCES += $(SOURCE_DIR)/parser/lexer.yy.c $(SOURCE_DIR)/parser/parser.tab.c
TO_CLEAN += $(SOURCE_DIR)/parser/lexer.yy.[ch] $(SOURCE_DIR)/parser/parser.tab.[ch]

%.yy.c: %.l
	flex --outfile=$*.yy.c --header-file=$*.yy.h $<

%.tab.c: %.y
	bison -Wall --output=$*.tab.c --header=$*.tab.h $<
# ==

include build.mk

