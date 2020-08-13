ifdef TEST
    TEST_SOURCES:= test/test_$(TEST).c
	TEST_BINS   := build/test
	SINGLE_TEST := 1
else
    TEST_SOURCES:= $(wildcard test/test_*.c)
	TEST_BINS   := $(patsubst test/%.c, build/%, $(TEST_SOURCES))
endif

TEST            ?= $*
CFLAGS          += -Isrc "-DTEST=\"$(TEST)\""

ifdef HEADLESS
    CFLAGS      += -DHEADLESS
endif

test: lib modules $(TEST_BINS)
	@for test_bin in $(TEST_BINS); do \
		./$$test_bin || exit 1;\
	done || true

test-watch: buildfs
	nodemon --delay 0.5 \
		-w include -w src -w test \
		-e .c,.h,.mk \
		-x "make --no-print-directory test || false"

ifdef SINGLE_TEST
build/test: test/test.c $(TEST_SOURCES) $(OBJECTS)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -o $@ $^ $(LDFLAGS)

else
# multiple test mode
$(TEST_BINS): build/test_%: test/test.c test/test_%.c $(OBJECTS)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -o $@ $^ $(LDFLAGS)
endif

.PHONY: test test-watch