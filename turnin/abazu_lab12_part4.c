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
 *  https://drive.google.com/file/d/1TTyFBK09ikluSbBErjSQjqAwtZnLcp-q/view?usp=sharing
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "tasks.h"
#include "timer.h"

typedef enum ledDisplay_states {init_ledD, wait_ledD} ledDisplay;
typedef enum ledShiftTick_states {wait_ledS, up, down, left, right, buttonPress} ledShift;


int ledDisplayTick (int state);
int ledShiftTick (int state);

int main(void) {
    DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks) / sizeof(task*);

    task1.state = init_ledD;
    task1.period = 1;
    task1.elapsedTime = task1.period;
    task1.TickFct = &ledShiftTick;

    task2.state = wait_ledS;
    task2.period = 1;
    task2.elapsedTime = task2.period;
    task2.TickFct = &ledDisplayTick;

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
unsigned char row[3] = {0xF7, 0xFB, 0xFD};
unsigned char column[3] = {0x3C, 0x24, 0x3C};

//-----------------------------------------

int ledDisplayTick(int state) {
    static unsigned char counter;
    static unsigned char r, c;

    switch (state) {
        case init_ledD: state = wait_ledD;
        counter = 0;
        break;

        case wait_ledD:
        state = wait_ledD;
        break;

    }

    switch (state) {
        case init_ledD:
        counter = 0;
        break;

        case wait_ledD:
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

int ledShiftTick(int state) {
    // 0001 = up, 0010 = down, 0100 = left, 1000 = right.
    unsigned char dPad = ~PINA & 0x0F;

    switch (state) {
        case wait_ledS:
            switch (dPad) {
                case 0x01: state = up; break;
                case 0x02: state = down; break;
                case 0x04: state = left; break;
                case 0x08: state = right; break;

                default :  state = wait_ledS; break;
            }
            break;

        case up:
            switch (dPad) {
                case 0x01: state = buttonPress; break;
                case 0x02: state = down; break;
                case 0x04: state = left; break;
                case 0x08: state = right; break;

                default:  state = wait_ledS; break;
            }
            break;

        case down:
            switch (dPad) {
                case 0x01: state = up; break;
                case 0x02: state = buttonPress; break;
                case 0x04: state = left; break;
                case 0x08: state = right; break;

                default:  state = wait_ledS; break;
            }
            break;

        case left:
            switch (dPad) {
                case 0x01: state = up; break;
                case 0x02: state = down; break;
                case 0x04: state = buttonPress; break;
                case 0x08: state = right; break;

                default:  state = wait_ledS; break;
            }
            break;

        case right:
            switch (dPad) {
                case 0x01: state = up; break;
                case 0x02: state = down; break;
                case 0x04: state = left; break;
                case 0x08: state = buttonPress; break;

                default:  state = wait_ledS; break;
            }
            break;

        case buttonPress:
            if (dPad) { state = buttonPress; }
            else { state = wait_ledS; }
            break;

    }

    switch (state) {
        case wait_ledS: break;

        case up:
            if (row[2] != 0xFE) {
                row[0] = (row[0] >> 1) | 0x80;
                row[1] = (row[1] >> 1) | 0x80;
                row[2] = (row[2] >> 1) | 0x80;
            }
            break;

        case down:
            if (row[0] != 0xEF) {
                row[0] = (row[0] << 1) | 0x01;
                row[1] = (row[1] << 1) | 0x01;
                row[2] = (row[2] << 1) | 0x01;
            }
            break;

        case left:
            if (column[0] != 0xF0) {
                column[0] = column[0] << 1;
                column[1] = column[1] << 1;
                column[2] = column[2] << 1;
            }
            break;

        case right:
            if (column[2] != 0x0F) {
                column[0] = column[0] >> 1;
                column[1] = column[1] >> 1;
                column[2] = column[2] >> 1;
            }
            break;

        case buttonPress: break;
    }


    return state;
}
