
# == Build type
BUILD ?= debug

export BUILD
# ==

# == Directories
ROOT_DIR   ?= $(shell git rev-parse --show-toplevel || realpath .)
BASE_DIR   ?= $(abspath .)
BUILD_DIR  ?= $(BASE_DIR)/build
SOURCE_DIR ?= $(BASE_DIR)/src
# ==

# == Common config
.SILENT:
.SECONDARY:
.SECONDEXPANSION:
# ==

# == Common rules

%/:
	mkdir -p $@

