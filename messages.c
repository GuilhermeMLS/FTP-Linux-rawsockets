#include "messages.h"

unsigned char calculateCRC(char *msg)
{
    char crc = 0;
    char *msg_char = (char *)msg;

    char *msg_calc = calloc(TAM_DADOS + 8, sizeof(char));
    for (int i = 0; i < TAM_DADOS; i++)
    {
        msg_calc[i] = msg_char[i];
    }

    short polynomial = CRC_POL; //<< (sizeof(short) - 4);
    printf("%hu\n", polynomial);
    short  r = 0;
    for (unsigned int i = 0; i < TAM_DADOS ; i++)
    {
        r = ((short)(msg_calc[i]) +( r )) % polynomial;
        (msg_calc)[i] /= polynomial;
        // for (int j = 0; j < sizeof(char); j++)
        // {
        // }
    }
    // char *msg = calloc(TAM_DADOS, sizeof(char));

    for (int i = 0; i < TAM_DADOS; i++)
    {
        printf("%X", msg_char[i]);
    }
    // printf("\n\n");

    // for (int i = 0; i < TAM_DADOS; i++)
    // {
    //     printf("%d", msg_calc[i]);
    // }
    printf("\n%X\n",r);

    return crc;
}

// int checkCRC(mensagem *msg)
// {
//     unsigned char crc = calculaCRC(msg);

// #ifdef DEBUG
//     printf("%d == %d :: %d\n", crc, msg->crc, (crc == msg->crc));
// #endif
//     return (crc == msg->crc);
// }