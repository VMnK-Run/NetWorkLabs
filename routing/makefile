SRC_DIR := src

SRC := $(wildcard $(SRC_DIR)/*.c)
CFLAGS = -Wall -g

all: $(SRC)
	gcc $(CFLAGS) $(SRC) -o routing

.PHONY: clean
clean:
	del -f routing.exe
