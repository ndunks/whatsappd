TEST_SOURCES    := $(wildcard test/*.c)
TEST_OBJECTS    := $(patsubst %.c, build/%.o, $(TEST_SOURCES))
TEST_BINS       := $(filter-out build/test, $(patsubst build/test/%.o, build/%, $(TEST_OBJECTS)))
MKDIRS          += build/test
OBJECTS_NO_MAIN := $(filter-out build/whatsappd.o, $(OBJECTS))


test: $(TEST_BINS)
	@for test_bin in $(TEST_BINS); do \
		./$$test_bin ;\
	done

test-watch:
	nodemon --delay 0.5 \
		-w libs -w src -w test \
		-e .c,.h,.mk -V \
		-x "make --no-print-directory test || false"

$(TEST_BINS): build/test_%: build/test/test_%.o build/test/test.o $(OBJECTS_NO_MAIN)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(TEST_OBJECTS): build/test/%.o: test/%.c
	$(CC) $(CFLAGS) -Isrc "-DTEST=\"$*\"" -c -o $@ $<

.PHONY: test test-watch