TEST_SOURCES    := $(wildcard test/test_*.c)
TEST_OBJECTS    := $(patsubst %.c, build/%.o, $(TEST_SOURCES))
TEST_BINS       := $(patsubst test/%.c, build/%, $(TEST_SOURCES))
OBJECTS_NO_MAIN := $(filter-out build/whatsappd.o, $(OBJECTS))

MKDIRS          += build/test

test: $(LIB_OBJECTS) $(OBJECTS_NO_MAIN) $(TEST_BINS)
	@for test_bin in $(TEST_BINS); do \
		./$$test_bin ;\
	done

test-watch:
	nodemon --delay 0.5 \
		-w libs -w src -w test \
		-e .c,.h,.mk -V \
		-x "make --no-print-directory test || false"

$(TEST_BINS): build/test_%: test/test.c build/test/test_%.o $(OBJECTS_NO_MAIN) $(BUILD_LIB)
	$(CC) $(CFLAGS) -Isrc "-DTEST=\"$*\"" $(LDFLAGS) -o $@ $^

$(TEST_OBJECTS): build/%.o: %.c
	$(CC) $(CFLAGS) -Isrc "-DTEST=\"$*\"" -c -o $@ $<

.PHONY: test test-watch