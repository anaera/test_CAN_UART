  
#ifndef XC_MAIN_H
#define	XC_MAIN_H

#include <xc.h> 

typedef union {
    uint8_t allFlags;

    struct {
        uint8_t hardSystem : 1; //1ms
        uint8_t hardApplic : 1; //100ms
        uint8_t appFlag : 1;
        uint8_t gapFlag : 1;
    };
} tmr_t;
#endif	/* XC_MAIN_H */

