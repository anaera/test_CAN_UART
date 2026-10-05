/*
 * File:   main.c
 * Author: Chiper
 *
 * Created on 3 сентября 2026 г., 23:55
 */

#include "config.h"
#include "initChip.h"
#include "timers.h"
#include "UCAN_driver.h"
#include "UART_driver.h"

void main(void) {
    uint8_t sec = 10;
    //uint8_t state, error;
    NOP();
    initChip();
    initDriverUCAN();
    initDriverUART();
    //setEcho();
    
    RA0_SetDigitalOutput(); //RECV_STEP
    RA1_SetDigitalOutput(); //SEND_STEP
    RC0_SetDigitalOutput(); //SKIP_STEP
    RC1_SetDigitalOutput(); //INT
    RC2_SetDigitalOutput(); //TMR0_GAP

    systemStart();

    while (1) {
        //		CLRWDT();
        recvPackUCAN();
            //sendPackUCAN();
        if (getSystemTick()) {
            NOP();
            clrSystemTick();
        }
        if (getTimeTick()) {
            NOP();
            clrTimeTick();
            sendPackUCAN();
        }
    }
}
