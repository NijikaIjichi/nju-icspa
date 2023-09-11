#include "x86.h"

void yield();

static volatile uint32_t jiffy = 0;

void timer_event() {
    ++jiffy;
}

void keyboard_event() { /* Do nothing */ }

void main_loop() {
    uint32_t i = 0;
    while (i < 20) {
        i++;
        Log("Hello from user proc for the %dth time! %d jiffy in this proc.", 
            i, jiffy);
        yield();
    }
    Log("Bye from user proc~");
}
