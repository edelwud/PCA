#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#include "com.c"

#define BASE 0x3f8

typedef unsigned int uint;

char* library[] = {
        "average.exe",
        "key.exe"
};

static int choice = -1;
static int choiceIndex = 0;
static int choiceCounter = 10;

unsigned int vector_base = 0xA0;

union REGS temp;

void consoleLog(char symbol) {
    temp.h.ah = 0x05;
    temp.h.cl = symbol;
    temp.h.ch = 0;
    int86(0x16, &temp, &temp);
}

void interrupt newRTC() {
    if (choice != -1) {
        if (choiceCounter == 10) {
            if (choiceIndex == 0) {
                consoleLog(13);
            }
            if (strlen(library[choice]) != choiceIndex) {
                consoleLog(library[choice][choiceIndex++]);
            } else {
                consoleLog(13);
                choice = -1;
            }
        } else {
            choiceCounter--;
            if (choiceCounter == 0)
                choiceCounter = 10;
        }
    }
    int86(0x08, &temp, &temp);
}

void interrupt newKeyboard() {
    int86(0x09, &temp, &temp);
}

int main() {
    char *request;
    int i;

    printf("Waiting for connection...\n");

    com_initializer(BASE);
    request = com_receive_string(BASE);

    if (strcmp(request, "show") != 0) {
        return 0;
    }
    com_send_string(library[0], BASE);
    com_send_string(library[1], BASE);
    com_send_string("completed", BASE);

    printf("Executable files loaded\n");

    com_initializer(BASE);
    request = com_receive_string(BASE);
    choice = request[0] - '0' - 1;

    printf("Your choice: %d; %s", choice + 1, library[choice]);

    setvect(vector_base + 0x00, newRTC);
    setvect(vector_base + 0x01, newKeyboard);

    disable();

    outp(0x20, 0x11);
    outp(0x21, vector_base);
    outp(0x21, 0x04);
    outp(0x21, 0x01);

    enable();

    temp.h.ah = 0x31;
    temp.h.al = 0x00;
    temp.x.dx = 0x0FFFF;
    int86(0x31, &temp, &temp);

    return 0;
}