BUILD_DIR    := $(PWD)/build
SOURCES      := $(wildcard src/*.c)
OBJECTS      := $(patsubst src/%.c, build/%.o, $(SOURCES))
BUILD_BIN    := build/whatsappd
BUILD_LIB    := build/libs.a
LIB_OBJECTS  :=
MKDIRS       := build/libs

LDFLAGS      += -lcurl
CFLAGS       += -std=c11


ifneq (,$(findstring test,$(MAKECMDGOALS)))
    include test/test.mk
endif

include libs/libs.mk

$(shell test -d build || mkdir -p $(MKDIRS))

all: $(BUILD_BIN)

run: all
	./$(BUILD_BIN)

$(BUILD_BIN): $(OBJECTS) $(BUILD_LIB)

$(OBJECTS) : build/%.o : src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_LIB)(build/libs/*.o):
	$(info Creating $@)

watch:
	nodemon --delay 0.5 -w src -e .c,.h -V -x "make run || false"

clean:
	@test -d build && find build/ -type f -delete

makedir:

.PHONY: all run watch clean

