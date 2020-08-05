BUILD_DIR       := $(PWD)/build
BUILD_BIN       := build/whatsappd
MKDIRS          := build
CFLAGS          += -Wall -D_GNU_SOURCE -std=c11 -Iinclude
LDFLAGS         := -Lbuild

SOURCES         := $(wildcard src/*.c)
OBJECTS         := $(patsubst src/%.c, build/%.o, $(SOURCES))

MODULES         := $(shell find src/* -type d)
MODULES_LIB     := build/modules.a
MODULES_SOURCES := $(foreach f, $(MODULES), $(wildcard $(f)/*.c))
MODULES_OBJECTS := $(patsubst src/%.c, build/%.o, $(MODULES_SOURCES))
MKDIRS          += $(patsubst src/%, build/%, $(MODULES))

.DEFAULT_GOAL := all

# $(info MKDIRS: $(MKDIRS))
# $(info MODULES: $(MODULES))
# $(info SOURCES: $(MODULES_SOURCES))
# $(info OBJECTS: $(MODULES_OBJECTS))

include lib/lib.mk

ifneq (,$(findstring test,$(MAKECMDGOALS)))
    include test/test.mk
    LDFLAGS += -lmbedtls -lmbedcrypto -lpthread
    export LD_LIBRARY_PATH := build/lib/mbedtls/library;
    MBEDTLS_CFLAGS += -DTEST
    SHARED := 1
else
    LDFLAGS += -l:modules.a -l:libmbedtls.a -l:libmbedcrypto.a -l:libmbedx509.a -lpthread
endif

# define MODULE_template =
# build/src_$(1).o: $(wildcard src/$(1)/*.c)
# 	$(CC) -c $(CFLAGS) $$^ -o $$@
# endef

$(foreach d, $(MKDIRS), $(shell test -d $(d) || mkdir -p $(d)))

# $(foreach m, $(MODULES), $(eval $(call MODULE_template,$(m))))

all: lib $(BUILD_BIN)

run: all
	./$(BUILD_BIN)

$(BUILD_BIN): $(OBJECTS) $(MODULES_LIB)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJECTS) $(MODULES_OBJECTS) : build/%.o : src/%.c
	$(CC) -c -o $@ $(CFLAGS) $<

$(MODULES_LIB): $(MODULES_OBJECTS)
	$(info archiving $?)
	@$(AR) cr $@ $?

modules: $(MODULES_LIB)

watch: buildfs
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

.PHONY: all run watch clean buildfs modules

