/*
 * File:   timers.c
 * Author: Chiper
 *
 * Created on 16 сентября 2026 г., 21:43
 */
#include "timers.h"

system_tick_t tick;

//------------------------------------------------------------------------------
// TMR1 -  100 миллисекунд 
// предделитель - 8
// делитель - 50000 (0xC350) регистр(дополнение до 0xFFFF) ->0x3CAF
// 4мгц / 8 = 500000, 500000 / 50000 = 10 
// итого 1/10 = 100 миллисек
//------------------------------------------------------------------------------

#define _TMR1H 0x3C
#define _TMR1L 0xAF

static void initTMR1(void)
{
	//init_TMR1
	//T1GSS T1G_pin; TMR1GE disabled; T1GTM disabled; T1GPOL low; T1GGO done; T1GSPM disabled; 
	T1GCON = 0x00;
	//	CCP1CON = 0x00;
	////!!!!!!!!!!!!!!!!!!!!!!!!!! pic16F1824 разремить !!
	//	CCP2CON = 0x00;
	TMR1H = _TMR1H; //0xF8 см.ISR();
	TMR1L = _TMR1L; //0x30 см.ISR();
	// T1CKPS 1:8; T1OSCEN disabled; nT1SYNC synchronize; TMR1CS FOSC/4; TMR1ON disabled; 
	T1CON = 0x30;
	//Устанавливаем приориет прерывания   
	//   IPR1bits.TMR1IP = 1; //HI priority
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// TMR2 -  1 миллисекунда 
// предделитель - 16
// делитель - 250 (0xFA)  регистр(дополнение до 0xFF) ->0x05
// постделитель - 10 (0x0A)
// 4мгц/16 = 250000, 125000/250 = 1000 1000/10 =1000
// итого 1/1000 = 1 миллисек
//------------------------------------------------------------------------------
//постделитель 1 ->0(000 0)000
#define T2OUTPS 0x00 // 0000 0000
//предделитель 16-> 0000 00(10)
#define T2CKPS  0x02 // 0000 0010

#define _TMR2 0x05

static void initTMR2(void)
{
	T2CON = T2OUTPS | T2CKPS;
	TMR2 = _TMR2;
}
//------------------------------------------------------------------------------

uint8_t getSystemTick(void)
{
	return tick.hardSystem;
}

void setSystemTick(void)
{
	TMR2 = _TMR2;
	tick.hardSystem = 1;
}

void clrSystemTick(void)
{
	tick.hardSystem = 0;
}

uint8_t getTimeTick(void)
{
	return tick.hardTime;
}

void setTimeTick(void)
{
	TMR1H = _TMR1H;
	TMR1L = _TMR1L;
	tick.hardTime = 1;
}

void clrTimeTick(void)
{
	tick.hardTime = 0;
}

void systemStart(void)
{
	initTMR1();
	initTMR2();
	
	T1CONbits.TMR1ON = 1; //старт таймер TMR1
	//Разрешаем прерывание от Таймера 1
	PIR1bits.TMR1IF = 0;
	PIE1bits.TMR1IE = 1;
	
	T2CONbits.TMR2ON = 1; //старт таймер TMR2
	//Разрешаем прерывание от Таймера 2
	PIR1bits.TMR2IF = 0;
	PIE1bits.TMR2IE = 1;

	INTCONbits.PEIE = 1; //Разрешаем прерывания от переферии
	INTCONbits.GIE = 1; //разрешаетвысокоприоритетные прерывания
	//RCONbits.IPEN = 1; //разрешаем приоритет прерываний
	//INTCONbits.GIEL = 0; //разрешает низкоприоретееные прерывания  
}