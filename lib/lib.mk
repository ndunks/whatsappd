MKDIRS      += build/lib/openssl build/lib/util
LIB_OBJECTS := $(patsubst %.c, build/%.o, $(wildcard lib/util/*.c))
BUILD_LIB   := build/lib/util.a
CFLAGS      += -Ilib/util -Ilib/openssl-1.1.1g/include -Ibuild/lib/openssl/include
LDFLAGS     += -Lbuild/lib/openssl

$(BUILD_LIB): $(LIB_OBJECTS)
	$(info Creating $@)
	$(AR) cr $@ $?

$(LIB_OBJECTS): build/lib/util/%.o: lib/util/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

openssl: lib/openssl-1.1.1g
	test -d build/lib/openssl || mkdir -p build/lib/openssl

ifeq (,$(wildcard build/lib/openssl/Makefile))
	cd build/lib/openssl && $(PWD)/lib/openssl-1.1.1g/config \
	-static no-tests no-engine no-ui-console \
	--prefix $(PWD)/build/lib/openssl

endif
	make -j$$(( $(shell nproc)/2 + 1 )) -C build/lib/openssl build_libs

lib/openssl-1.1.1g:
	wget -qO - https://www.openssl.org/source/openssl-1.1.1g.tar.gz | tar -C lib -xz

.PHONY: openssl