/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #12  Exercise #1
 *	Exercise Description: [Design a system where an illuminated row of the LED
    matrix can be shifted up or down based on button presses.]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "transmit_shift_data"
#include "timer.h"

typedef enum state_machine {init_sm, wait_sm};

int smTick (int state);

int main(void) {
    DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    task1.state = init_lc;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &lightControlTick;

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

int smTick(int state) {

    return state;
}
