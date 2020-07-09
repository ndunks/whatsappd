#include "whatsappd.h"
#include "wasocket.h"

#ifndef TEST

int main(int argc, char const *argv[])
{

    if (wasocket_connect())
    {
        err("Fail init socket");
        return 1;
    }

    return 0;
}

#endif