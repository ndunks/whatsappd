MKDIRS          += build/lib/openssl
LIB_OBJECTS     := $(patsubst %.c, build/%.o, $(wildcard lib/util/*.c))
BUILD_LIB       := build/lib.a

$(BUILD_LIB): $(LIB_OBJECTS)
	$(info Creating $@)
	$(AR) cr $@ $?

$(LIB_OBJECTS): build/lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

openssl: lib/openssl-1.1.1g
ifeq (,$(wildcard lib/openssl-1.1.1g/Makefile))
	lib/openssl-1.1.1g/config --prefix $(PWD)/build/lib/openssl
endif
	make -C lib/openssl-1.1.1g build_libs install

lib/openssl-1.1.1g:
	wget -qO - https://www.openssl.org/source/openssl-1.1.1g.tar.gz | tar -C lib -xz

.PHONY: openssl