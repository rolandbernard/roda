
# == Build type
BUILD ?= debug

export BUILD
# ==

# == Directories
ROOT_DIR      ?= $(shell git rev-parse --show-toplevel || realpath .)
BASE_DIR      ?= $(abspath .)
BUILD_DIR     ?= $(BASE_DIR)/build
# ==

# == Common config
.SILENT:
.SECONDEXPANSION:
# ==

# == Common rules

%/:
	mkdir -p $@

