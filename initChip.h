/* 
 * File:   initChip.h
 * Author: Chiper
 *
 * Created on 17 сентября 2026 г., 19:20
 */
 
#ifndef INITCHIP_H
#define	INITCHIP_H
#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>
#include <stdint.h>

//#define INT_RA2
#define PWM_TMR6_RC3_P1C

inline static void initWDT(void);
inline static void initCPU(void);
inline static void initPORTA(void);
inline static void initPORTC(void);
void initChip(void);

#endif	/* INITCHIP_H */

