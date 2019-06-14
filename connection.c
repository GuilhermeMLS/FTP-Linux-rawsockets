#include "connection.h"

int RawSocketConnection(char *device)
{
    int socketNumber;
    struct ifreq ir;
    struct sockaddr_ll address;
    struct packet_mreq mr;

    // cria socket
    socketNumber = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (socketNumber == -1) {
        printf("Socket error\n");
        exit(-1);
    }

    // eth0
    memset(&ir, 0, sizeof(struct ifreq));
    memcpy(ir.ifr_name, device, sizeof(device));
    if (ioctl(socketNumber, SIOCGIFINDEX, &ir) == -1) {
        printf("ioctl() Error\n");
        exit(-1);
    }

    // IP do dispositivo
    memset(&address, 0, sizeof(address));
    address.sll_family = AF_PACKET;
    address.sll_protocol = htons(ETH_P_ALL);
    address.sll_ifindex = ir.ifr_ifindex;
    if (bind(socketNumber, (struct sockaddr *)&address, sizeof(address)) == -1) {
        printf("bind() Error\n");
        exit(-1);
    }

    // modo promíscuo
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(socketNumber, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        printf("setsockopt() Error\n");
        exit(-1);
    }

    return socketNumber;
}
