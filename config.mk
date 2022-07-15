
# == Build type
BUILD ?= debug

export BUILD
# ==

# == Directories
ROOT_DIR   ?= $(shell git rev-parse --show-toplevel || realpath . || true)
BASE_DIR   ?= $(abspath .)
BUILD_DIR  ?= $(BASE_DIR)/build
SOURCE_DIR ?= $(BASE_DIR)/src
# ==

# == Some info
GIT_URL  ?= $(shell git remote get-url origin || true)
GIT_HEAD ?= $(shell git rev-parse HEAD || true)
# ==

# == Common config
ifneq ($(VERBOSE),yes)
.SILENT:
endif
.SECONDARY:
.SECONDEXPANSION:

EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
# ==

# == Common rules

%/:
	mkdir -p $@

