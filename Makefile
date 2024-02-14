# Main idea taken from here --> very nice explanation:
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories
CC = gcc
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin# or . if you want it in the current directory

EXE := $(BIN_DIR)/run_main
TEST := $(BIN_DIR)/test
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -I/opt/homebrew/include -I./include    # D_THREAD_SAFE was proposed by sdl2-config --cflags
CFLAGS   := -Wall -g
LDFLAGS  := -L/opt/homebrew/lib/
LDLIBS   := -llsl -lSDL2 -lSDL2_image -lSDL2_ttf


.PHONY: all clean  # PHONY to tell that these are not expected to be created as files/paths

all: $(EXE)

$(EXE): $(OBJ_DIR)/main.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ -v

# $(EXE): $(OBJ) | $(BIN_DIR) 
# 	echo "exe for $^"
# 	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c | $(OBJ_DIR)
	echo "obj for $^ to $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ -v


$(TEST): $(OBJ_DIR)/test.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ -v

$(OBJ_DIR)/test.o: $(SRC_DIR)/test.c | $(OBJ_DIR)
	echo "obj for $^ to $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ -v


$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# build and run
run: $(EXE)
	$(EXE)

test: $(TEST)
	$(TEST)

clean: 
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
