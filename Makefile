CC = gcc
CFLAGS = -Wall -Wextra -O3 -I./include

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source discovery
# Find all sources in src/ EXCEPT main.c (server entry point)
COMMON_SOURCES = $(filter-out $(SRC_DIR)/main.c, $(shell find $(SRC_DIR) -name '*.c'))
COMMON_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(COMMON_SOURCES))

# Targets
SERVER_TARGET = $(BUILD_DIR)/network_server
CLIENT_TEST_TARGET = $(BUILD_DIR)/client_test

# Main execution entry points
SERVER_MAIN = $(SRC_DIR)/main.c
CLIENT_TEST_MAIN = $(TEST_DIR)/client_test.c

# Phony targets
.PHONY: all clean directories

all: directories $(SERVER_TARGET) $(CLIENT_TEST_TARGET)

directories:
	@mkdir -p $(BUILD_DIR)

# Build the Network Server
$(SERVER_TARGET): $(COMMON_OBJECTS) $(SERVER_MAIN)
	$(CC) $(CFLAGS) -o $@ $^

# Build the Test Client
$(CLIENT_TEST_TARGET): $(COMMON_OBJECTS) $(CLIENT_TEST_MAIN)
	$(CC) $(CFLAGS) -o $@ $^

# Compile generic object files from src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)