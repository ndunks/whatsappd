MKDIRS      += tmp build/lib/mbedtls build/lib/util
LIB_OBJECTS := $(patsubst %.c, build/%.o, $(wildcard lib/util/*.c))
BUILD_LIB   := build/lib/util.a
CFLAGS      += -Ilib/util -Ilib/mbedtls/include
LDFLAGS     += -Lbuild/lib/mbedtls/library

MBEDTLS_VER := 2.23.0

$(BUILD_LIB): $(LIB_OBJECTS)
	$(info Creating $@)
	$(AR) cr $@ $?

$(LIB_OBJECTS): build/lib/util/%.o: lib/util/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

mbedtls: lib/mbedtls

# Forced with: make mbedtls REMAKE=1
# ifdef REMAKE
# 	rm -rf build/lib/mbedtls/*
# endif

ifeq (,$(wildcard build/lib/mbedtls/Makefile$(REMAKE)))
	cd build/lib/mbedtls && cmake \
	-DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF \
	-DUSE_SHARED_MBEDTLS_LIBRARY=ON \
	$(PWD)/$^
endif
	make -j$$(( $(shell nproc)/2 + 1 )) -C build/lib/mbedtls

lib/mbedtls: tmp/mbedtls-$(MBEDTLS_VER).tar.gz
	tar -C lib -xzf $^
	mv lib/mbedtls-* $@

tmp/mbedtls-%.tar.gz:
	@cd tmp && wget -q https://github.com/ARMmbed/mbedtls/archive/mbedtls-$*.tar.gz

.PHONY: mbedtls