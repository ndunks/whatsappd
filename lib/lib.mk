MKDIRS          += build/lib
LIB_OBJECTS     := $(patsubst %.c, build/%.o, $(wildcard lib/*.c))
BUILD_LIB       := build/lib.a

$(BUILD_LIB): $(LIB_OBJECTS)
	$(info Creating $@)
	$(AR) cr $@ $?

$(LIB_OBJECTS): build/lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
