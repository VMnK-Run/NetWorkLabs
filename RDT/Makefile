SRC_DIR := src
OBJ_DIR := obj

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(OBJ_DIR)/StopAndWait.o
BIN := SW.exe
CC := gcc
CPPFLAGS := -Iinclude
CFLAGS := -g -Wall

default: all
all : SW

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

SW: $(OBJ_DIR)/StopAndWait.o
	$(CC) -Werror $^ -o $@

$(OBJ_DIR):
	mkdir $@

.PHONY: clean
# 这是在Windows系统下的命令
clean:
	del $(BIN)
	rmdir /s/q $(OBJ_DIR)
