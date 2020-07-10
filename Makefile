BUILD_DIR    := $(PWD)/build
SOURCES      := $(wildcard src/*.c)
OBJECTS      := $(patsubst src/%.c, build/%.o, $(SOURCES))
BUILD_BIN    := build/whatsappd
MKDIRS       := build
CFLAGS       += -Wall -D_GNU_SOURCE -std=c11

.DEFAULT_GOAL := all

include lib/lib.mk


ifneq (,$(findstring test,$(MAKECMDGOALS)))
    include test/test.mk
    LDFLAGS += -lssl -lcrypto -lpthread
else
    LDFLAGS += -l:libssl.a -l:libcrypto.a -lpthread -ldl -lc
endif

$(foreach d, $(MKDIRS), $(shell test -d $(d) || mkdir -p $(d)))


all: $(BUILD_BIN)

run: all
	./$(BUILD_BIN)

$(BUILD_BIN): $(OBJECTS) $(BUILD_LIB)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJECTS) : build/%.o : src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

watch:
	nodemon --delay 0.5 -w Makefile -w src -e .c,.h,.mk -x "make run || false"

clean:
	@test -d build && rm -rf build/*

buildfs:
ifeq (,$(shell grep $(PWD)/build /proc/mounts))
	@echo "Make build as tempfs"
	@test -d build && rm -rf build/* || true
	sudo mount -t tmpfs none ./build
else
	@echo "Already mounted"
endif

.PHONY: all run watch clean buildfs

