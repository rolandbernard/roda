
# == Progress
ifndef ECHO
TOTAL   := \
	$(shell $(MAKE) $(MAKECMDGOALS) --no-print-directory -nrRf $(firstword $(MAKEFILE_LIST)) \
		ECHO="__HIT_MARKER__" BUILD=$(BUILD) | grep -c "__HIT_MARKER__")
TLENGTH := $(shell expr length $(TOTAL))
COUNTER  = $(words $(HIDDEN_COUNT))
COUNTINC = $(eval HIDDEN_COUNT := x $(HIDDEN_COUNT))
PERCENT  = $(shell expr $(COUNTER) '*' 100 / $(TOTAL))
ECHO     = $(COUNTINC)printf "[%*i/%i](%3i%%) %s\n" $(TLENGTH) $(COUNTER) $(TOTAL) $(PERCENT)
endif
# ==

# == Directories
OBJECT_DIR := $(BUILD_DIR)/$(BUILD)/obj
BINARY_DIR := $(BUILD_DIR)/$(BUILD)/bin
# ==

# == Common Flags
SANITIZE := address,leak,undefined
# SANITIZE ?= thread,undefined
WARNINGS := -Wall -Wextra -Wno-unused-parameter

CCFLAGS.gdb 	+= -O0 -g -DDEBUG
LDFLAGS.gdb 	+= -O0 -g
CCFLAGS.debug   += -O0 -g -fsanitize=$(SANITIZE) -DDEBUG
LDFLAGS.debug   += -O0 -g -fsanitize=$(SANITIZE)
CCFLAGS.release += -O3
LDFLAGS.release += -O3

CCFLAGS += $(CCFLAGS.$(BUILD)) $(WARNINGS) -MMD -MP -I$(SOURCE_DIR)
CCFLAGS += $(foreach SWITCH, $(SWITCHES), -D$(shell echo $(SWITCH) | tr '[:lower:]' '[:upper:]'))
CCFLAGS += $(foreach SWITCH, $(filter-out $(SWITCHES), $(ALL_SWITCHES)), -DNO$(shell echo $(SWITCH) | tr '[:lower:]' '[:upper:]'))
CCFLAGS += $(foreach SWITCH, $(SWITCHES), $(CCFLAGS.$(SWITCH)))
LDFLAGS += $(LDFLAGS.$(BUILD))
LDFLAGS += $(foreach SWITCH, $(SWITCHES), $(LDFLAGS.$(SWITCH)))
# ==

# == Files
PATTERNS         := *.c *.S
$(foreach SWITCH, $(ALL_SWITCHES), \
	$(eval SWITCH_SOURCES.$(SWITCH) \
		+= $(foreach PATTERN, $(PATTERNS), $(shell find $(SOURCE_DIR) -type f -path '$(SOURCE_DIR)*/$(SWITCH)/*' -name '$(PATTERN)'))) \
	$(eval SWITCH_SOURCES.$(SWITCH) := $(sort $(SWITCH_SOURCES.$(SWITCH)))))
$(foreach TARGET, $(TARGETS), \
	$(eval TARGET_SOURCES.$(TARGET) \
		+= $(foreach PATTERN, $(PATTERNS), $(shell find $(SOURCE_DIR) -type f -path '$(SOURCE_DIR)*/$(TARGET)/*' -name '$(PATTERN)'))) \
	$(eval TARGET_SOURCES.$(SWITCH) := $(sort $(TARGET_SOURCES.$(SWITCH)))))
ALL_SOURCES      += $(foreach PATTERN, $(PATTERNS), $(shell find $(SOURCE_DIR) -type f -name '$(PATTERN)'))
ALL_SOURCES      := $(sort $(ALL_SOURCES))
SWITCH_SOURCES   := $(foreach SWITCH, $(ALL_SWITCHES), $(SWITCH_SOURCES.$(SWITCH)))
ENABLED_SOURCES  := $(filter-out $(SWITCH_SOURCES), $(ALL_SOURCES)) $(foreach SWITCH, $(SWITCHES), $(SWITCH_SOURCES.$(SWITCH)))
DISABLED_SOURCES := $(filter-out $(ENABLED_SOURCES), $(ALL_SOURCES))
TARGET_SOURCES   := $(foreach TARGET, $(TARGETS), $(TARGET_SOURCES.$(TARGET)))
COMMON_SOURCES   := $(filter-out $(TARGET_SOURCES), $(ENABLED_SOURCES))
$(foreach TARGET, $(TARGETS), \
	$(eval TARGET_SOURCES.$(TARGET) = $(filter-out $(DISABLED_SOURCES), $(TARGET_SOURCES.$(TARGET)))))
$(foreach TARGET, $(TARGETS), \
	$(eval TARGET_OBJECTS.$(TARGET) = $(patsubst $(SOURCE_DIR)/%, $(OBJECT_DIR)/%.o, $(TARGET_SOURCES.$(TARGET)))))
ALL_OBJECTS      := $(patsubst $(SOURCE_DIR)/%, $(OBJECT_DIR)/%.o, $(ALL_SOURCES))
OBJECTS          := $(patsubst $(SOURCE_DIR)/%, $(OBJECT_DIR)/%.o, $(COMMON_SOURCES))
DEPENDENCIES     := $(ALL_OBJECTS:.o=.d)
BINARYS          := $(foreach TARGET, $(TARGETS), $(BINARY_DIR)/$(TARGET))
# ==

# == Other
TO_CLEAN += $(BUILD_DIR)
# ==

.PHONY: build clean

build: $(TARGETS)
	@$(FINISHED)
	@$(ECHO) "Build successful."

$(TARGETS): $(BINARY_DIR)/$$@

$(BINARYS): $(BINARY_DIR)/%: $(OBJECTS) $$(TARGET_OBJECTS.$$*) $(LINK_SCRIPT) | $$(dir $$@)
	@$(ECHO) "Building $@"
	$(if $(LD.$*), $(LD.$*), $(LD)) $(LDFLAGS) $(LDFLAGS.$*) -o $@ $(OBJECTS) $(TARGET_OBJECTS.$*) \
		$(if $(LINK_SCRIPT.$*), -T$(LINK_SCRIPT.$*), $(if $(LINK_SCRIPT), -T$(LINK_SCRIPT)))
	@$(CHANGED)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/% $(MAKEFILE_LIST) | $$(dir $$@)
	@$(ECHO) "Building $@"
	$(CC) $(CCFLAGS) $(CCFLAGS.$*) -c -o $@ $<

clean:
	$(RM) -rf $(TO_CLEAN)
	@$(ECHO) "Cleaned generated files."

-include $(DEPENDENCIES)

