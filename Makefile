CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lcurl -lcjson
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

EXE = $(BIN_DIR)/jarvis

OBJ = $(OBJ_DIR)/test_os.o $(OBJ_DIR)/os_utils.o $(OBJ_DIR)/network.o $(OBJ_DIR)/payload_handler.o

all: $(EXE)

$(EXE): $(OBJ)
		@mkdir -p $(BIN_DIR)
		$(CC) -o $(EXE) $(OBJ) $(LDFLAGS)
		@echo "Created the executable successfuly!"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
		@mkdir -p $(OBJ_DIR)
		$(CC) $(CFLAGS) -c $< -o $@

run: $(EXE)
		@clear
		@./$(EXE)

clean:
		@rm -rf $(OBJ_DIR) $(BIN_DIR)
		@echo "Removed all objects and binaries"