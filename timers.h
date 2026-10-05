

#ifndef TIMERS_H
#define	TIMERS_H
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

void systemStart(void);
uint8_t getSystemTick(void);
void setSystemTick(void);
void clrSystemTick(void);
uint8_t getTimeTick(void);
void setTimeTick(void);
void clrTimeTick(void);

static void initTMR1(void);
static void initTMR2(void);

typedef struct {
    uint8_t hardSystem : 1; //1ms
    uint8_t hardTime : 1; //100ms
    uint8_t appFlag : 1;
    uint8_t gapFlag : 1;
} system_tick_t;

#endif	/* TIMERS_H */

