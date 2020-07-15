#include <stdio.h>
#include <color.h>
#include <helper.h>
#include "wasocket.h"

int whatsappd_init(int argc, char const *argv[])
{
    TRY(crypto_init());
    return 0;

CATCH:
    return 1;
}

void whatsappd_free()
{
    crypto_free();
}

#ifndef TEST

int main(int argc, char const *argv[])
{
    TRY(whatsappd_init(argc, argv));
    if (wasocket_connect())
    {
        err("Fail init socket");
        return 1;
    }

    return 0;
    
CATCH:
    return 1;
}

#endif