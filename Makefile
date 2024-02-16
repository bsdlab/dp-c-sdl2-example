# Main idea taken from here --> very nice explanation:
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories
CC = gcc
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin# or . if you want it in the current directory

# The final executables
PARADIGM := $(BIN_DIR)/main_paradigm
TEST := $(BIN_DIR)/test
SERVER := $(BIN_DIR)/server
ECHO_SERVER := $(BIN_DIR)/echo_server

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -I/opt/homebrew/include -I./include   # D_THREAD_SAFE was proposed by sdl2-config --cflags
CFLAGS   := -Wall -g
LDFLAGS  := -L/opt/homebrew/lib/
LDLIBS   := -llsl -lSDL2 -lSDL2_image -lSDL2_ttf

.PHONY: all clean paradigm test server paths  # PHONY to tell that these are not expected to be created as files/paths

all: $(SERVER)

$(PARADIGM): $(OBJ_DIR)/main_paradigm.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/main_paradigm.o: $(SRC_DIR)/examples/main_paradigm.c | $(OBJ_DIR)
	@echo ">>> obj for $^ to $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


# ------- Compile the test file for using SDL to display an image
$(TEST): $(OBJ_DIR)/test.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/test.o: $(SRC_DIR)/examples/test.c | $(OBJ_DIR)
	@echo ">>> obj for $^ to $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# ------- Compile the server as an example

$(ECHO_SERVER): $(OBJ_DIR)/echo_server.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@  

$(OBJ_DIR)/echo_server.o: $(SRC_DIR)/examples/echo_server.c | $(OBJ_DIR)
	@echo ">>> obj for $^ to $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@  

# ------- Paradigm and the server
$(SERVER):$(OBJ_DIR) $(OBJ) | $(BIN_DIR)  
	$(CC) $(LDFLAGS)  $(filter-out $(OBJ_DIR),$^) $(LDLIBS) -o $@  

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@  

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# build and run the paradigm without server
paradigm: $(PARADIGM)
	$(PARADIGM)

# build and run rendering an image
test: $(TEST)
	$(TEST)

# build and run the echo server example
echo: $(ECHO_SERVER)
	$(ECHO_SERVER)

# build and run the actual paradigm dareplane server
server: $(SERVER)
	$(SERVER)

paths:
	@echo "SRC: $(SRC)"
	@echo "OBJ: $(OBJ)"

clean: 
	$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

# -include $(OBJ:.o=.d)
