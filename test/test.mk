ifdef TEST
    TEST_SOURCES:= test/test_$(TEST).c
else
    TEST_SOURCES:= $(wildcard test/test_*.c)
endif

TEST            ?= $*
TEST_CFLAGS      = -Isrc "-DTEST=\"$(TEST)\""
TEST_OBJECTS    := $(patsubst %.c, build/%.o, $(TEST_SOURCES))
TEST_BINS       := $(patsubst test/%.c, build/%, $(TEST_SOURCES))
OBJECTS_NO_MAIN := $(filter-out build/whatsappd.o, $(OBJECTS))
MKDIRS          += build/test

test: $(TEST_BINS)
	@for test_bin in $(TEST_BINS); do \
		./$$test_bin ;\
	done

test-watch:
	nodemon --delay 0.5 \
		-w lib -w src -w test \
		-e .c,.h,.mk \
		-x "make --no-print-directory test || false"

$(TEST_BINS): build/test_%: test/test.c build/test/test_%.o $(OBJECTS_NO_MAIN) $(BUILD_LIB)
	@$(CC) $(CFLAGS) $(TEST_CFLAGS) $(LDFLAGS) -o $@ $^

$(TEST_OBJECTS): build/test/test_%.o: test/test_%.c
	@$(CC) $(CFLAGS) $(TEST_CFLAGS) -c -o $@ $<

.PHONY: test test-watch