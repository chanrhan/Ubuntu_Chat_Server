# Compiler and flags
CC = gcc
CFLAGS = -g -ggdb3 -O0 -Wall -fno-omit-frame-pointer \
         -Iinclude -Iinclude/server -Iinclude/client \
         -fstack-protector -fno-stack-protector `mysql_config --cflags`
# JWT support
JWT_CFLAGS = $(shell pkg-config --cflags libjwt)
JWT_LIBS   = $(shell pkg-config --libs   libjwt)

# Directories
SRC_DIR     = src
BUILD_DIR   = build
OBJ_DIR     = $(BUILD_DIR)/obj
INCLUDE_DIR = include

# Subdirectories
client_DIR = $(SRC_DIR)/client
SERVER_DIR = $(SRC_DIR)/server
CMD_DIR    = $(SERVER_DIR)/cmd

# Common sources
COMMON_SRCS1 = $(SRC_DIR)/socket_common.c
COMMON_SRCS2 = $(SRC_DIR)/common.c

# Source lists
client_SRCS = $(wildcard $(client_DIR)/*.c) $(COMMON_SRCS1) $(COMMON_SRCS2)
SERVER_SRCS = $(wildcard $(SERVER_DIR)/*.c) \
              $(wildcard $(CMD_DIR)/*.c) \
              $(COMMON_SRCS1) $(COMMON_SRCS2)

# Object files
client_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(client_SRCS))
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))

# Targets
client_TARGET = client
SERVER_TARGET = server

# Default target
all: check_dependencies $(client_TARGET) $(SERVER_TARGET)

# Dependency check
check_dependencies:
	@if ! pkg-config --exists gtk+-3.0; then \
	  echo "GTK+-3.0 not found. Installing..."; \
	  sudo apt-get update && sudo apt-get install -y libgtk-3-dev; \
	else \
	  echo "GTK+-3.0 is installed."; \
	fi
	@if ! command -v mysql_config >/dev/null 2>&1; then \
	  echo "MySQL client library not found. Installing..."; \
	  sudo apt-get install -y libmysqlclient-dev; \
	else \
	  echo "MySQL client library is installed."; \
	fi
	@if ! pkg-config --exists libjwt; then \
	  echo "libjwt not found. Installing..."; \
	  sudo apt-get update && sudo apt-get install -y libjwt-dev; \
	else \
	  echo "libjwt is installed."; \
	fi

# Create build and object directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/client
	mkdir -p $(OBJ_DIR)/server
	mkdir -p $(OBJ_DIR)/server/cmd

# Compile sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(JWT_CFLAGS) -c $< `pkg-config --cflags gtk+-3.0` -o $@

# Link client
$(client_TARGET): $(client_OBJS)
	$(CC) $(CFLAGS) $^ `pkg-config --cflags --libs gtk+-3.0` `mysql_config --libs` -o $@

# Link server (with JWT support)
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $^ `mysql_config --libs` $(JWT_LIBS) -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
