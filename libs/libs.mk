LDFLAGS         += -lssl -lcrypto -lpthread
CFLAGS          += -Ilibs/libwsclient
MKDIRS          += build/libs/libwsclient
LIBWSCLIENT_OBJ := $(patsubst %.c, build/%.o, $(wildcard libs/libwsclient/*.c))
LIB_OBJECTS     += $(LIBWSCLIENT_OBJ)


$(LIBWSCLIENT_OBJ): build/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
