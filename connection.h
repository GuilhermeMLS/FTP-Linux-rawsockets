#ifndef __CONNECTION
    #define __CONNECTION
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <net/ethernet.h>
    #include <linux/if_packet.h>
    #include <linux/if.h>
    #include <stdlib.h>
    #include <string.h>
    #include <arpa/inet.h>
    #include <stdio.h>

    int rawSocketConnection(char *device);

#endif
