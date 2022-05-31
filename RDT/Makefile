SRC_DIR := src
OBJ_DIR := obj

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(OBJ_DIR)/StopAndWait.o GoBackN.o
BIN := SW SW.exe GBN.exe
CC := gcc
CPPFLAGS := -Iinclude
CFLAGS := -g -Wall

default: all
all : SW GBN

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

SW: $(OBJ_DIR)/StopAndWait.o
	$(CC) -Werror $^ -o $@

GBN: $(OBJ_DIR)/GoBackN.o
	$(CC) -Werror $^ -o $@

$(OBJ_DIR):
	mkdir $@

.PHONY: clean

clean:
#	del $(BIN) 
#	rmdir /s/q $(OBJ_DIR)
	$(RM) $(OBJ) $(BIN) log/*.txt
	$(RM) -rf $(OBJ_DIR) 
