CFLAGS = -std=c11 -Wall -Wextra -Werror -g -I include
LDFLAGS = -lsqlite3 -lpthread -ldl -lm

UNAME_S = $(shell uname -s)
GIT_HOOKS := .git/hooks/applied

# strtok_r is provided by POSIX.1c-1995 and POSIX.1i-1995, however, with
# the POSIX_C_SOURCE=1 on Mac OS X is corresponding to the version of
# 1988 which is too old (defined in sys/cdefs.h)
CFLAGS += -D_POSIX_C_SOURCE=199506L

OUT = bin
EXEC = $(OUT)/facebooc
OBJS = \
	src/kv.o \
	src/response.o \
	src/template.o \
	src/main.o \
	src/bs.o \
	src/request.o \
	src/list.o \
	src/models/like.o \
	src/models/account.o \
	src/models/connection.o \
	src/models/session.o \
	src/models/post.o \
	src/http/header.o \
	src/http/body.o \
	src/http/cookies.o \
	src/http/helper.o \
	src/http/query.o \
	src/http/http.o \
	src/server.o

deps := $(OBJS:%.o=%.o.d)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -DDEBUG -o $@ -MMD -MF $@.d -c $<

$(EXEC): $(OBJS) $(GIT_HOOKS)
	mkdir -p $(OUT)
	$(CC) $(OBJS) -fsanitize=address -g -o $@ $(LDFLAGS)

all: $(GIT_HOOKS) $(EXEC)
run: $(EXEC)
	@echo "Starting Facebooc service..."
	@./$(EXEC) $(port)
release: $(OBJS)
	mkdir -p $(OUT)
	$(CC) $(CFLAGS) -O3 -s -o $(EXEC) $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(EXEC) $(deps)
distclean: clean
	$(RM) db.sqlite3

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

-include $(deps)
