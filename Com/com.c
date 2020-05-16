#include <time.h>

#define BAUD 12
#define PACKET_END 0
#define MAX_BUF_SIZ 512
#define TIMEOUT 0.020

char com_receive(int base) {
    return inp(base);
}

char* com_receive_string(int base) {
    int i = 0;
    char buffer[MAX_BUF_SIZ];

    time_t control = 0, checker = 0;

    do {
        while (!com_check_rhr(base)) {
            checker = time(0);
            if (control) {
                if (difftime(checker, control) > TIMEOUT) {
                    buffer[i] = '\0';
                    return buffer;
                }
            }
        };
        buffer[i] = com_receive(base);
        control = time(0);
    } while (buffer[i++] != PACKET_END);

    return buffer;
}

/*
 * server, client
 *
 * server -> client
 * client ждем пока информация придет
 * когда приходит считываем из порта
 *
 */

void com_initializer(int base) {
    unsigned int lcr;

    lcr = inp(base + 0x03);
    outp(base + 0x03, lcr | 0x80); /* setting DLAB bit */

    outp(base, 0x0C); /* setting freq divider */
    outp(base + 0x01, 0x00);

    outp(base + 0x03, lcr & 0x7f); /* resetting DLAB bit */

    outp(base + 0x01, 0x00); /* disable interrupts */

    outp(base + 0x03, 0x1b); /* parity check, 1 stop bit, byte size - 8bit */
    outp(base + 0x04, 0x00); /* modem control register: DTR - 0, RTS - 0 */
}

void com_send(char symbol, int base) {
    unsigned char lsr;
    unsigned char mcr = inp(base + 0x04); /* modem control register */
    outp(base + 0x04, mcr | 0x02); /* setting RTS */

    do lsr = inp(base + 0x05) & 0x40; /* transmitter status */
    while (lsr != 0x40); /* transmitter empty */

    outp(base, symbol); /* write symbol */

    while (com_check_tbe(base)); /* wait for transmitter ready */
    outp(base + 0x04, mcr | 0xfd); /* resetting RTS */
}

void com_send_string(char *string, int base) {
    do {
        com_initializer(base);
        com_send(*string, base);
    } while (*(string++));
}

int com_check_rhr(int base) {
    unsigned char lsr = inp(base + 0x05);
    return (lsr & 0x01) == 0x01; /* check if receiver ready */
}

int com_check_tbe(int base) {
    unsigned char lsr = inp(base + 0x05);
    return ((lsr & 0x20) >> 5) == 0x01; /* check if transmitter ready */
}