
# == Progress
ifndef ECHO
ifneq ($(PROGRESS),no)
COUNTER  = $(words $(HIDDEN_COUNT))
COUNTINC = $(eval HIDDEN_COUNT := x $(HIDDEN_COUNT))
PERCENT  = $(shell expr $(COUNTER) '*' 100 / $(TOTAL))
ECHO     = $(COUNTINC)printf "[%*i/%i](%3i%%) %s\n" $(TLENGTH) $(COUNTER) $(TOTAL) $(PERCENT)

TOTAL   := \
	$(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory -nrRf $(firstword $(MAKEFILE_LIST)) \
		ECHO="__HIT_MARKER__" BUILD=$(BUILD) SWITCHES="$(SWITCHES)" TARGETS="$(TARGETS)" | grep -c "__HIT_MARKER__")
TLENGTH := $(shell expr length $(TOTAL))
else
ECHO    := echo
endif
endif
# ==

# == Directories
BUILD_NAME ?= build-$(BUILD)_$(subst $(SPACE),_,$(SWITCHES))
OBJECT_DIR := $(BUILD_DIR)/$(BUILD_NAME)/obj
BINARY_DIR := $(BUILD_DIR)/$(BUILD_NAME)/bin
# ==

# == Common Flags
SANITIZE := address,leak,undefined
# SANITIZE ?= thread,undefined
WARNINGS := -Wall -Wextra -Wno-unused-parameter

CFLAGS.debug    += -O0 -g -fsanitize=$(SANITIZE) -DDEBUG
LDFLAGS.debug   += -O0 -g -fsanitize=$(SANITIZE)
CFLAGS.release  += -O3
LDFLAGS.release += -O3

CFLAGS.gdb       += -O0 -g -DDEBUG
LDFLAGS.gdb      += -O0 -g
CFLAGS.profile   += -O3 -g
LDFLAGS.profile  += -O3 -g
CFLAGS.coverage  += -O0 -g -fprofile-instr-generate -fcoverage-mapping -DDEBUG -DCOVERAGE
LDFLAGS.coverage += -O0 -g -fprofile-instr-generate -fcoverage-mapping

ifeq ($(BUILD),coverage)
CC := clang
LD := clang++
endif

CFLAGS  += $(CFLAGS.$(BUILD)) $(WARNINGS) -MMD -MP -I$(SOURCE_DIR)
CFLAGS  += $(foreach SWITCH, $(SWITCHES), -D$(shell echo $(SWITCH) | tr '[:lower:]' '[:upper:]'))
CFLAGS  += $(foreach SWITCH, $(filter-out $(SWITCHES), $(ALL_SWITCHES)), -DNO$(shell echo $(SWITCH) | tr '[:lower:]' '[:upper:]'))
CFLAGS  += $(foreach SWITCH, $(SWITCHES), $(CFLAGS.$(SWITCH)))
LDFLAGS += $(LDFLAGS.$(BUILD))
LDFLAGS += $(foreach SWITCH, $(SWITCHES), $(LDFLAGS.$(SWITCH)))
LDLIBS  += $(LDLIBS.$(BUILD))
LDLIBS  += $(foreach SWITCH, $(SWITCHES), $(LDLIBS.$(SWITCH)))
# ==

# == Files
PATTERNS         := *.c *.S
$(foreach SWITCH, $(ALL_SWITCHES), \
	$(eval SWITCH_SOURCES.$(SWITCH) \
		+= $(foreach PATTERN, $(PATTERNS), $(shell find $(SOURCE_DIR) -type f -path '$(SOURCE_DIR)*/$(SWITCH)/*' -name '$(PATTERN)'))) \
	$(eval SWITCH_SOURCES.$(SWITCH) := $(sort $(SWITCH_SOURCES.$(SWITCH)))))
$(foreach TARGET, $(ALL_TARGETS), \
	$(eval TARGET_SOURCES.$(TARGET) \
		+= $(foreach PATTERN, $(PATTERNS), $(shell find $(SOURCE_DIR) -type f -path '$(SOURCE_DIR)*/$(TARGET)/*' -name '$(PATTERN)'))) \
	$(eval TARGET_SOURCES.$(SWITCH) := $(sort $(TARGET_SOURCES.$(SWITCH)))))
ALL_SOURCES      += $(foreach PATTERN, $(PATTERNS), $(shell find $(SOURCE_DIR) -type f -name '$(PATTERN)')) $(GEN_SOURCES)
ALL_SOURCES      := $(sort $(ALL_SOURCES))
SWITCH_SOURCES   := $(foreach SWITCH, $(ALL_SWITCHES), $(SWITCH_SOURCES.$(SWITCH)))
ENABLED_SOURCES  := $(filter-out $(SWITCH_SOURCES), $(ALL_SOURCES)) $(foreach SWITCH, $(SWITCHES), $(SWITCH_SOURCES.$(SWITCH)))
DISABLED_SOURCES := $(filter-out $(ENABLED_SOURCES), $(ALL_SOURCES))
TARGET_SOURCES   := $(foreach TARGET, $(ALL_TARGETS), $(TARGET_SOURCES.$(TARGET)))
COMMON_SOURCES   := $(filter-out $(TARGET_SOURCES), $(ENABLED_SOURCES))
$(foreach TARGET, $(ALL_TARGETS), \
	$(eval TARGET_SOURCES.$(TARGET) = $(filter-out $(DISABLED_SOURCES), $(TARGET_SOURCES.$(TARGET)))))
$(foreach TARGET, $(ALL_TARGETS), \
	$(eval TARGET_OBJECTS.$(TARGET) = $(patsubst $(SOURCE_DIR)/%, $(OBJECT_DIR)/%.o, $(TARGET_SOURCES.$(TARGET)))))
ALL_OBJECTS      := $(patsubst $(SOURCE_DIR)/%, $(OBJECT_DIR)/%.o, $(ALL_SOURCES))
OBJECTS          := $(patsubst $(SOURCE_DIR)/%, $(OBJECT_DIR)/%.o, $(COMMON_SOURCES))
DEPENDENCIES     := $(ALL_OBJECTS:.o=.d)
BINARYS          := $(foreach TARGET, $(ALL_TARGETS), $(BINARY_DIR)/$(TARGET))
# ==

# == Other
TO_CLEAN += $(BUILD_DIR)
# ==

.PHONY: build clean

build: $(TARGETS)
	@$(FINISHED)
	$(RM) -r $(BUILD_DIR)/$(BUILD) $(BUILD_DIR)/last
	ln -s $(BUILD_DIR)/$(BUILD_NAME) $(BUILD_DIR)/$(BUILD)
	ln -s $(BUILD_DIR)/$(BUILD_NAME) $(BUILD_DIR)/last
	@$(ECHO) "Build successful."

$(ALL_TARGETS): $(BINARY_DIR)/$$@

$(BINARYS): $(BINARY_DIR)/%: $(OBJECTS) $$(TARGET_OBJECTS.$$*) $(LINK_SCRIPT) | $$(dir $$@)
	@$(ECHO) "Building $@"
	$(if $(LD.$*), $(LD.$*), $(LD)) $(LDFLAGS) $(LDFLAGS.$*) $(OBJECTS) $(TARGET_OBJECTS.$*) \
		$(LDLIBS) $(LDLIBS.$*) -o $@ \
		$(if $(LINK_SCRIPT.$*), -T$(LINK_SCRIPT.$*), $(if $(LINK_SCRIPT), -T$(LINK_SCRIPT)))
	@$(CHANGED)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/% $(MAKEFILE_LIST) $(GEN_SOURCES) | $$(dir $$@)
	@$(ECHO) "Building $@"
	$(CC) $(CFLAGS) $(CFLAGS.$*) -c -o $@ $<

clean:
	$(RM) -r $(TO_CLEAN)
	@$(ECHO) "Cleaned generated files."

-include $(DEPENDENCIES)

