/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #12  Exercise #1
 *	Exercise Description: [Design a system where an illuminated row of the LED
    matrix can be shifted up or down based on button presses.]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link:
 *  https://drive.google.com/file/d/1sHmR76C3v8xwuKJUOQ-1Fmf9QSliXlbG/view?usp=sharing
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "timer.h"

typedef enum ledShiftTick_states {init_ledS, wait_ledS, up, down, buttonPress} ledShift;

int ledShiftTick (int state);

int main(void) {
    DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    task1.state = init_ledS;
    task1.period = 100;
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
    static unsigned char row;
    static unsigned char column;

    unsigned char A0 = ~PINA & 0x01;
    unsigned char A1 = ~PINA & 0x02;

    switch (state) {
        case init_ledS: state = wait_ledS;
            row = 0xFB;
            column = 0xFF;
            break;

        case wait_ledS:
            if (A0 && !A1) { state = up; }
            else if (!A0 && A1) { state = down; }
            else { state = wait_ledS; }
            break;

        case up:
            if (A0 && !A1) { state = buttonPress; }
            else if (!A0 && A1) { state = down; }
            else { state = wait_ledS; }
            break;

        case down:
            if (!A0 && A1) { state = buttonPress; }
            else if (A0 && !A1) { state = up; }
            else { state = wait_ledS; }
            break;

        case buttonPress:
            if (!A0 && !A1) { state = wait_ledS; }
            else { state = buttonPress; }
            break;

    }

    switch (state) {
        case init_ledS:
            // Start with the middle row lit
            row = 0xFB;
            column = 0xFF;
            break;

        case wait_ledS: break;

        case up:
            if (row != 0xFE) {
                // Shifts the low bit up one and rewrites the resulting 0 into
                // a 1
                row = (row >> 1) | 0x80;
                // ex: 0xFB => 111 1011
                // -> 0111 1101 => 1111 1101
            }
            break;

        case down:
            if (row != 0xEF) {
                // opposite of up case
                row = (row << 1) | 0x01;
                // ex: 0xFB -> 1111 1011
                // -> 1111 0110 => 1111 0111
            }
            break;

        case buttonPress: break;
    }

    PORTC = column;
    PORTD = row;

    return state;
}
