# Simplecoin Makefile
# Builds a single binary (Simplecoind) from all source files

# ============================================================================
# Configuration
# ============================================================================

# Project settings
PROJECT_NAME := Simplecoind
VERSION := 1.0.0

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIR := include
TEST_DIR := tests

# Output
TARGET := $(BIN_DIR)/$(PROJECT_NAME)

# Compiler settings
CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11 -O2 -g
CPPFLAGS := -I$(INCLUDE_DIR) -I$(SRC_DIR) -DVERSION=\"$(VERSION)\"
LDFLAGS := -lsodium -lpthread -lm

# Debug build
DEBUG_CFLAGS := -O0 -g3 -DDEBUG -fsanitize=address -fsanitize=undefined
DEBUG_LDFLAGS := $(LDFLAGS) -fsanitize=address -fsanitize=undefined

# Release build
RELEASE_CFLAGS := -O3 -march=native -DNDEBUG -flto
RELEASE_LDFLAGS := $(LDFLAGS) -flto

# Installation
PREFIX := /usr/local
INSTALL_BIN := $(PREFIX)/bin
INSTALL_MAN := $(PREFIX)/share/man/man1

# ============================================================================
# Source Files Discovery
# ============================================================================

# Find all .c files in src/ and subdirectories
SOURCES := $(shell find $(SRC_DIR) -name '*.c')

# Generate object file paths in build directory (preserving structure)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Dependency files
DEPS := $(OBJECTS:.o=.d)

# Test sources
TEST_SOURCES := $(shell find $(TEST_DIR) -name '*_test.c' 2>/dev/null)
TEST_OBJECTS := $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/test/%.o)
TEST_BINS := $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BIN_DIR)/%)

# ============================================================================
# Build Targets
# ============================================================================

.PHONY: all clean install uninstall test help debug release

# Default target
all: $(TARGET)

# Link final binary
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $(TARGET)..."
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# ============================================================================
# Directory Creation
# ============================================================================

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ============================================================================
# Build Variants
# ============================================================================

# Debug build with sanitizers
debug: CFLAGS := $(DEBUG_CFLAGS)
debug: LDFLAGS := $(DEBUG_LDFLAGS)
debug: clean $(TARGET)
	@echo "Debug build complete with sanitizers enabled"

# Optimized release build
release: CFLAGS := $(RELEASE_CFLAGS)
release: LDFLAGS := $(RELEASE_LDFLAGS)
release: clean $(TARGET)
	@strip $(TARGET)
	@echo "Release build complete and stripped"

# ============================================================================
# Testing
# ============================================================================

# Build and run all tests
test: $(TEST_BINS)
	@echo "Running tests..."
	@for test in $(TEST_BINS); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "All tests passed!"

# Build individual test binaries
$(BIN_DIR)/%_test: $(BUILD_DIR)/test/%_test.o $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) | $(BIN_DIR)
	@echo "Building test: $@..."
	@$(CC) $^ -o $@ $(LDFLAGS)

# Compile test source files
$(BUILD_DIR)/test/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling test: $<..."
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# ============================================================================
# Installation
# ============================================================================

install: release
	@echo "Installing $(PROJECT_NAME) to $(INSTALL_BIN)..."
	@install -d $(INSTALL_BIN)
	@install -m 755 $(TARGET) $(INSTALL_BIN)
	@echo "Installation complete!"
	@echo "Run '$(PROJECT_NAME)' to start"

uninstall:
	@echo "Uninstalling $(PROJECT_NAME)..."
	@rm -f $(INSTALL_BIN)/$(PROJECT_NAME)
	@echo "Uninstall complete"

# ============================================================================
# Maintenance
# ============================================================================

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

# Show build information
info:
	@echo "=== Simplecoin Build Information ==="
	@echo "Project:       $(PROJECT_NAME) v$(VERSION)"
	@echo "Compiler:      $(CC)"
	@echo "CFLAGS:        $(CFLAGS)"
	@echo "LDFLAGS:       $(LDFLAGS)"
	@echo "Sources:       $(words $(SOURCES)) files"
	@echo "Target:        $(TARGET)"
	@echo "Build dir:     $(BUILD_DIR)"
	@echo "==================================="

# Display help
help:
	@echo "Simplecoin Makefile - Available targets:"
	@echo ""
	@echo "  make              - Build default release"
	@echo "  make all          - Same as 'make'"
	@echo "  make debug        - Build with debug symbols and sanitizers"
	@echo "  make release      - Build optimized release version"
	@echo "  make test         - Build and run all tests"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make install      - Install to $(PREFIX)"
	@echo "  make uninstall    - Remove installed files"
	@echo "  make info         - Show build configuration"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Build options:"
	@echo "  PREFIX=/path      - Change installation prefix (default: /usr/local)"
	@echo "  CC=compiler       - Change compiler (default: gcc)"
	@echo "  CFLAGS='flags'    - Add custom compiler flags"
	@echo ""
	@echo "Examples:"
	@echo "  make release              - Optimized build"
	@echo "  make debug                - Debug build with sanitizers"
	@echo "  make PREFIX=~/.local      - Install to home directory"
	@echo "  make test                 - Run test suite"

# ============================================================================
# Dependency Tracking
# ============================================================================

# Include dependency files (auto-generated by -MMD flag)
-include $(DEPS)

# ============================================================================
# Phony Targets
# ============================================================================

.DEFAULT_GOAL := all