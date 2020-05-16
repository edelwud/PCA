#include <dos.h>
#include <stdio.h>
#include <conio.h>

#include "com.c"

#define BASE 0x2f8

typedef unsigned int uint;

int main() {
    char* result;
    char choice[1];
    int index = 0;
    com_initializer(BASE);
    com_send_string("show", BASE);

    com_initializer(BASE);

    do {
        result = com_receive_string(BASE);
        if (strcmp(result, "completed") == 0)
            break;
        printf("%d. %s\n", ++index, result);
    } while(1);

    choice[0] = getch();
    com_send_string(choice, BASE);

    return 0;
}