/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #12  Exercise #2
 *	Exercise Description: [Design a system where an illuminated column of the LED
    matrix can be shifted up or down based on button presses.]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link:
 *  https://drive.google.com/file/d/1GfxOrM87OdYqTDTh4Q2KbFpyP0ELCilh/view?usp=sharing
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "timer.h"

typedef enum ledShiftTick_states {init_ledS, wait_ledS} ledShift;

int ledShiftTick (int state);

int main(void) {
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    task1.state = init_ledS;
    task1.period = 1;
    task1.elapsedTime = task1.period;
    task1.TickFct = &ledShiftTick;

    unsigned long GCD = tasks[0]->period;
    for (unsigned char i = 0; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();

    while (1) {
        for (unsigned char i = 0; i < numTasks; i++) {
            if (tasks[i]->elapsedTime == tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }

        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}

// Shared variables

//-----------------------------------------

int ledShiftTick(int state) {
    unsigned char row[3] = {0xF7, 0xFB, 0xFD};
    unsigned char column[3] = {0x3C, 0x24, 0x3C};
    static unsigned char counter;
    static unsigned char r, c;

    switch (state) {
        case init_ledS: state = wait_ledS;
            counter = 0;
            break;

        case wait_ledS:
            state = wait_ledS;
            break;

    }

    switch (state) {
        case init_ledS:
            counter = 0;
            break;

        case wait_ledS:
            r = row[counter % 3];
            c = column[counter % 3];
            ++counter;
            if (counter > 3) {
                counter = 0;
            }
            break;

    }

    PORTC = c;
    PORTD = r;

    return state;
}
