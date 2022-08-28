ifeq ($(strip $(CFLAGS)),)
CFLAGS = -std=gnu11 -Wall -Wextra -Werror
endif

ifeq ($(strip $(LDFLAGS)),)
LDFLAGS = -lsqlite3 -lpthread -ldl -lm -lc -lssl -lcrypto
endif

DEBUG = -DDEBUG -g -fsanitize=address

SRC = $(shell find . -type f -name "*.[ch]")
OBJ = $(SRC:%.c=%.o)

.PHONY: all clean test

all: $(OBJ)

http/%.o: http/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -o $@ $<

clean:
	$(RM) $(OBJ)

test: $(OBJ)
	@echo Do testing...
	@./driver.py
	@echo done
