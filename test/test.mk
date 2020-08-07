ifdef TEST
    TEST_SOURCES:= test/test_$(TEST).c
else
    TEST_SOURCES:= $(wildcard test/test_*.c)
endif

TEST            ?= $*
CFLAGS          += -Isrc "-DTEST=\"$(TEST)\""
TEST_BINS       := $(patsubst test/%.c, build/%, $(TEST_SOURCES))

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

$(TEST_BINS): build/test_%: test/test.c test/test_%.c $(OBJECTS)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test test-watch