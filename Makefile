
include config.mk

# == Config
TARGETS := compiler
# ==

# == Tools
CC ?= gcc
LD := $(CC)
#==

include build.mk

