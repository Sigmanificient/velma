.SUFFIXES:

CC ?= gcc

SRC += $(wildcard *.c)

CFLAGS += -pedantic -O2
CFLAGS += -shared -fPIC

LDLIBS += -lpython3.11

OUT := vera.so

all: $(OUT)

$(OUT): $(SRC)
	$(CC) -o $@ $(CFLAGS) $(SRC) $(LDLIBS)

.PHONY: fclean
fclean:
	$(RM) $(OUT)

.PHONY: re
.NOTPARALLEL: re
re: fclean all

COMPILEDB_CMD ?= bear -- $(MAKE) -f $(firstword $(MAKEFILE_LIST)) re
COMPILEDB_JSON := compile_commands.json

.PHONY: compiledb
compiledb: $(COMPILEDB_JSON)

$(COMPILEDB_JSON): $(SRC)
	$(COMPILEDB_CMD)
