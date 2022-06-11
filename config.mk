
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

# == Common config
ifneq ($(VERBOSE),yes)
.SILENT:
endif
.SECONDARY:
.SECONDEXPANSION:
# ==

# == Common rules

%/:
	mkdir -p $@

