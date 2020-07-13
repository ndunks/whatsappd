MKDIRS         += tmp build/lib/mbedtls build/lib/util
LIB_OBJECTS    := $(patsubst %.c, build/%.o, $(wildcard lib/util/*.c))
BUILD_LIB      := build/lib/util.a
CFLAGS         += -Ilib/util -Ilib/mbedtls/include
LDFLAGS        += -Lbuild/lib/mbedtls/library

MBEDTLS_LIB    := build/lib/mbedtls/library/libmbedtls.a
MBEDTLS_VER    := 2.23.0
MBEDTLS_CFLAGS := -I$(PWD)/lib/util -DMBEDTLS_CONFIG_FILE='<config.h>'

build_lib: mbedtls $(BUILD_LIB)

$(BUILD_LIB): $(LIB_OBJECTS)
	$(info Creating $@)
	@$(AR) cr $@ $?

$(LIB_OBJECTS): build/lib/util/%.o: lib/util/%.c
	@$(CC) $(CFLAGS) -c -o $@ $<

mbedtls: lib/mbedtls build/lib/mbedtls/Makefile $(MBEDTLS_LIB)

$(MBEDTLS_LIB): lib/util/config.h
	make -j$$(( $(shell nproc)/2 + 1 )) \
		--always-make \
		CFLAGS="$(MBEDTLS_CFLAGS)" \
		-C build/lib/mbedtls
	cd build/lib/mbedtls/library && ls -alh  *.a | cut -d ' ' -f5-

build/lib/mbedtls/Makefile: lib/util/config.h
ifneq (,$(wildcard build/lib/mbedtls))
	rm -rf build/lib/mbedtls
endif
	mkdir -p build/lib/mbedtls
	cd build/lib/mbedtls && \
		CFLAGS="$(MBEDTLS_CFLAGS)" cmake \
		-DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF \
		-DUSE_SHARED_MBEDTLS_LIBRARY=ON \
		$(PWD)/lib/mbedtls

lib/mbedtls: tmp/mbedtls-$(MBEDTLS_VER).tar.gz
	tar -C lib -xzf $^
	mv lib/mbedtls-* $@

tmp/mbedtls-%.tar.gz:
	@cd tmp && wget -q https://github.com/ARMmbed/mbedtls/archive/mbedtls-$*.tar.gz

.PHONY: mbedtls build_lib