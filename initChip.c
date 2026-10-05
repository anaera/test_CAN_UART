/*
 * File:   initChip.c
 * Author: Chiper
 *
 * Created on 17 сентября 2026 г., 19:18
 */
#include "initChip.h"
//--------------------------------------------------------

inline static void initWDT(void)
{
	//init_WDT

	WDTCON = 0b00010010; //512ms
	//        WDTCONbits.SWDTEN=1;
}

inline static void initCPU(void)
{
	// SCS FOSC; SPLLEN disabled; IRCF 16MHz_HF; 
	OSCCON = 0x78;
	// TUN 0; 
	OSCTUNE = 0x00;
	// SBOREN disabled; 
	BORCON = 0x00;
}

inline static void initPORTA(void)
{
	//init_PORTA

	PORTA = 0x00;
	LATA = 0x00;
	ANSELA = 0x00; // all analog to digital I/O    
	TRISA = 0x3F; //all pins input
	INTCON = 0x00; //сброс флага INTF

}

inline static void initPORTC(void)
{
	//init_PORTC
	PORTC = 0x00;
	LATC = 0x00;
	ANSELC = 0x00; // all analog to digital I/O 
	TRISC = 0x3F; //all pins input
}
//---------------------------------------------------------------------
// Инициализация ECCP2
//---------------------------------------------------------------------

/*static void initECCP2(void)
{
	// RC3 выставляем на выход
	TRISCbits.TRISC3 = 0;
	// PM single; DCB 0x0; CCPM P2A: active high; P2B: active high; 
	CCP2CON = 0x0C;
	// CTSEL PWM2timer6; 
	CCPTMRS0 = 0x08;
	// CCPASE operating; CCPAS disabled; PSSAC low; PSSBD low; 
	CCP2AS = 0x0;
	// PRSEN manual_restart; PDC 0x0; 
	PWM2CON = 0x0;
	// STRSYNC start_at_begin; STRB P2B_to_port; STRA P2A_to_CCP2M; 
	PSTR2CON = 0x01;
	// CCPRL 255; 
	CCPR2L = 125;
	// CCPRH 0x0; 
	CCPR2H = 0x0;
}*/

void initChip(void)
{
	initCPU();
	initWDT();
	initPORTA();
	initPORTC();
	//initECCP2();
	return;
}
