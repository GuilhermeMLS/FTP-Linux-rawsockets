#include "connection.h"

int RawSocketConnection(char *device)
{
    int socket;
    struct ifreq ir;
    struct sockaddr_ll address;
    struct packet_mreq mr;

    // create socket
    socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (socket == -1) {
        printf("Socket error\n");
        exit(-1);
    }

    // eth0
    memset(&ir, 0, sizeof(struct ifreq));
    memcpy(ir.ifr_name, device, sizeof(device));
    if (ioctl(socket, SIOCGIFINDEX, &ir) == -1) {
        printf("ioctl() Error\n");
        exit(-1);
    }

    // device IP
    memset(&address, 0, sizeof(address));
    address.sll_family = AF_PACKET;
    address.sll_protocol = htons(ETH_P_ALL);
    address.sll_ifindex = ir.ifr_ifindex;
    if (bind(socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
        printf("bind() Error\n");
        exit(-1);
    }

    // promiscuous mode
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        printf("setsockopt() Error\n");
        exit(-1);
    }

    return socket;
}
