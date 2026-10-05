/* 
 * File:   CAN_driver.h
 * Author: Chiper
 *
 * Created on 15 сентября 2026 г., 0:18
 */

#ifndef CAN_DRIVER_H
#define	CAN_DRIVER_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
void initDriverUCAN(void);
void packSendSelect(void);
void sendPackUCAN(void);
void recvPackUCAN(void);

typedef union {
    uint8_t raw[11];

    struct {

        union {
            uint16_t canHead;

            struct {
                uint8_t hiHead;
                uint8_t loHead;
            };

            struct {
                uint8_t hiNode : 4;
                uint8_t cmd_can : 4;

                uint8_t lenData : 4;
                uint8_t rtr : 1;
                uint8_t loNode : 3;
            };
        };
        uint8_t data[8];
        uint8_t crc;

    };
} packCAN_t;

#endif	/* CAN_DRIVER_H */

