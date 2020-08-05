MKDIRS         += tmp build/lib/mbedtls

CFLAGS         += -Ilib/mbedtls/include
LDFLAGS        += -Lbuild/lib/mbedtls/library

MBEDTLS_LIB    := build/lib/mbedtls/library/libmbedtls.a
MBEDTLS_VER    := 2.23.0
MBEDTLS_CFLAGS := -I$(PWD)/include -DMBEDTLS_CONFIG_FILE='<config.h>'

LIBS           := $(MBEDTLS_LIB)

lib: mbedtls

mbedtls: lib/mbedtls build/lib/mbedtls/Makefile $(MBEDTLS_LIB)

$(MBEDTLS_LIB): include/config.h
	make -j$$(( $(shell nproc)/2 + 1 )) \
		--always-make \
		-C build/lib/mbedtls
	cd build/lib/mbedtls/library && ls -alh  *.a | cut -d ' ' -f5-

build/lib/mbedtls/Makefile: include/config.h
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
	tar -C lib -xzf $<
	mv lib/mbedtls-* $@

tmp/mbedtls-%.tar.gz:
	@cd tmp && wget -q https://github.com/ARMmbed/mbedtls/archive/mbedtls-$*.tar.gz

.PHONY: mbedtls lib