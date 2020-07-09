BUILD_DIR    := $(PWD)/build
SOURCES      := $(wildcard src/*.c)
OBJECTS      := $(patsubst src/%.c, build/%.o, $(SOURCES))
BUILD_BIN    := build/whatsappd
MKDIRS       := build
CFLAGS       := -Wall -D_GNU_SOURCE -std=c11 -Ilib
LDFLAGS      := -lssl -lcrypto -lpthread

.DEFAULT_GOAL := all

include lib/lib.mk

ifneq (,$(findstring test,$(MAKECMDGOALS)))
    include test/test.mk
endif

$(shell test -d build || mkdir -p $(MKDIRS))

all: $(BUILD_BIN)

run: all
	./$(BUILD_BIN)

$(BUILD_BIN): $(OBJECTS) $(BUILD_LIB)

$(OBJECTS) : build/%.o : src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

watch:
	nodemon --delay 0.5 -w Makefile -w src -e .c,.h,.mk -x "make run || false"

clean:
	@test -d build && rm -rf build

.PHONY: all run watch clean

