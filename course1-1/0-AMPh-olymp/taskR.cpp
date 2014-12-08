#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int main()
{
    char* nodename = nullptr;
    scanf ("%m[^\n]", &nodename);

    addrinfo hints;
    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET;

    addrinfo* info = nullptr;
    int result = getaddrinfo (nodename, nullptr, &hints, &info);
    assert (!result);
    assert (info->ai_family == AF_INET);

    sockaddr* addr = info->ai_addr;
    assert (addr->sa_family == AF_INET);

    sockaddr_in* addr_in = (sockaddr_in*)addr;

    char addrname[INET_ADDRSTRLEN];
    inet_ntop (AF_INET, &addr_in->sin_addr, addrname, INET_ADDRSTRLEN);

    printf ("%s\n", addrname);

    freeaddrinfo (info);
    free (nodename);
    return 0;
}
