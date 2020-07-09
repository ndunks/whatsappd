#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "whatsappd.h"
#include "wasocket.h"
static int ws_fd;

static int wasocket_init()
{
    struct addrinfo hints, *hostinfo, *ptr;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("web.whatsapp.com", "443", &hints, &hostinfo) != 0)
        die("Can't resolve host!");

    for (ptr = hostinfo; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((ws_fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
        {
            continue;
        }
        if (connect(ws_fd, ptr->ai_addr, ptr->ai_addrlen) == -1)
        {
            close(ws_fd);
            continue;
        }
        break;
    }
    freeaddrinfo(hostinfo);
    return ptr == NULL;
}

static int wasocket_handshake()
{
    return 0;
}

int wasocket_connect()
{
    return wasocket_init() || wasocket_handshake();
}

int wasocket_disconnect()
{
    return close(ws_fd);
}