/*
 * File:   isr_driver.c
 * Author: Chiper
 */

#include "UART_driver.h"
#include "UCAN_driver.h"
#include "timers.h"

void __interrupt() fullINT(void) {
    if (PIE1bits.RCIE) {
        if (PIR1bits.RCIF) {
            // флаг PIR1bits.RCIF сбрасывается автоматически при чтении RCREG
            engineUART();
        }
    }
 #ifdef INT_RA2   
    if (INTCONbits.INTE) {
        if (INTCONbits.INTF) {
            INTCONbits.INTE = 0; //запрещаем прерывание от INT0	
            INTCONbits.INTF = 0; //сбрасываем флаг  
            setStateReceive(); //переход в режим приема 
         }
    }
#else
	if (INTCONbits.IOCIE) {
		if (IOCAFbits.IOCAF3) {
            INTCONbits.IOCIE = 0;
            IOCAFbits.IOCAF3 = 0;
            INTCONbits.IOCIF = 0;
            setStateReceive(); //переход в режим приема
        }
    }
#endif

    if (INTCONbits.TMR0IE) {
        if (INTCONbits.TMR0IF) { //триггер на 200мкс(500мкс)
            INTCONbits.TMR0IF = 0;
            nextStepIDLE();
        }
    }
    if (PIE1bits.TMR2IE) {
        if (PIR1bits.TMR2IF) { //1мс
            PIR1bits.TMR2IF = 0;
            setSystemTick();
        }
    }
    if (PIE1bits.TMR1IE) {
        if (PIR1bits.TMR1IF) { //100мс
            PIR1bits.TMR1IF = 0;
            setTimeTick();
        }
    }
}
