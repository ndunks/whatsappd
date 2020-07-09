TEST_SOURCES := $(wildcard test/test_*.c)
TEST_BINS    := $(patsubst test/%.c, build/%, $(TEST_SOURCES))

test: $(TEST_BINS)
	for test_bin in $(TEST_BINS); do \
		./$$test_bin ;\
	done

test-watch:
	nodemon --delay 0.5 -i build -i .git -i tmp -e .c,.h -V -x "make test || false"

$(TEST_BINS): build/test_%: test/test_%.c
	$(CC) $(CFLAGS) -Isrc "-DTEST=\"$*\"" -o $@ $< test/test.c

.PHONY: test test-watch