SRC_DIR := src

SRC := $(wildcard $(SRC_DIR)/*.c)
CFLAGS = -Wall -g
CPPFLAGS := -I include

all: $(SRC)
	gcc $(CPPFLAGS) $(CFLAGS) $(SRC) -o routing

.PHONY: clean save run
clean:
	del -f routing.exe
	del -f result.txt

save:
	.\routing.exe > .\result.txt

run:
	.\routing.exe
