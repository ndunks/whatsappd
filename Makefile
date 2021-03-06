BUILD_DIR       := $(PWD)/build
BUILD_BIN       := build/whatsappd
MKDIRS          := build
CFLAGS          += -Wall -D_GNU_SOURCE -std=c11 -Iinclude
LDFLAGS         := -Lbuild

SOURCES         := $(wildcard src/*.c)
OBJECTS         := $(patsubst src/%.c, build/%.o, $(SOURCES))
BIN_DEPS        := $(OBJECTS)

MODULES         := $(shell find src/* -type d)
MODULES_SOURCES := $(foreach f, $(MODULES), $(wildcard $(f)/*.c))
MODULES_OBJECTS := $(patsubst src/%.c, build/%.o, $(MODULES_SOURCES))
MODULES_FLAGS   := 
MKDIRS          += $(patsubst src/%, build/%, $(MODULES))

.DEFAULT_GOAL := all

ifdef DEBUG
    CFLAGS += -g -DDEBUG=$(DEBUG)
endif

ifdef SAVE_MSG
    CFLAGS += "-DSAVE_MSG=\"$(SAVE_MSG)\""
endif

include lib/lib.mk

ifneq (,$(findstring test,$(MAKECMDGOALS)))
    include test/test.mk
    SHARED := 1
endif

ifdef SHARED
    MODULES_LIB    := build/libmodules.so
	LDFLAGS        += -lmodules -lmbedtls -lmbedcrypto -lpthread \
					  -Lbuild/lib/mbedtls/library
    MBEDTLS_CFLAGS += -DTEST
	MODULES_FLAGS  += -fPIC
    export LD_LIBRARY_PATH := build:build/lib/mbedtls/library
else
    MODULES_LIB    := build/libmodules.a
	BIN_DEPS       += $(MODULES_LIB)
    LDFLAGS        += -l:libmodules.a -l:libmbedtls.a -l:libmbedcrypto.a -l:libmbedx509.a -lpthread
endif

$(foreach d, $(MKDIRS), $(shell test -d $(d) || mkdir -p $(d)))

all: lib $(MODULES_LIB) $(BUILD_BIN)
ifdef SHARED
	$(info Done build shared binary)
endif

run: all
	./$(BUILD_BIN)

$(BUILD_BIN): $(BIN_DEPS)
	@$(CC) -o $@ $^ $(LDFLAGS)

$(OBJECTS) $(MODULES_OBJECTS) : build/%.o : src/%.c
	@$(CC) -c -o $@ $(CFLAGS) $(MODULES_FLAGS) $<

$(MODULES_LIB): $(MODULES_OBJECTS)
	$(info building $@)
ifdef SHARED
	$(CC) -shared $^ -o $@
else
	$(AR) cr $@ $?
endif

modules: $(MODULES_LIB)

watch: buildfs
	nodemon --delay 0.5 -w Makefile -w src -e .c,.h,.mk -x "make run || false"

# Clean build without lib
clean:
	@find build -mindepth 1 -maxdepth 1 -not -name lib -exec rm -rf \{\} \;

clean_all:
	@test -d build && rm -rf build/*

buildfs:
ifeq (,$(shell grep $(PWD)/build /proc/mounts))
	@echo "Make build as tempfs"
	@test -d build && rm -rf build/* || true
	sudo mount -t tmpfs none ./build
else
	@echo "Already mounted"
endif

.PHONY: all run watch clean clean_all buildfs modules

