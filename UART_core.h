/* 
 * File:   UART_driver.h
 * Author: Chiper
 *
 * Created on 4 сентября 2026 г., 23:59
 */

#ifndef UART_CORE_H
#define	UART_CORE_H
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "debug.h"

//#define _TMR0 0x90 //Константа на GAP_TIME 500мкс
#define _TMR0 0xCE //Константа на GAP_TIME 200мкс

/**
 * Машина состояний приёма/передачи:
 */
#define IDLE_STEP  0x00 // ожидание активности шины (INT0)
#define RECV_STEP  0x01 // приём пакета (выигран арбитраж или пассивный узел)
#define SEND_STEP  0x02 // передача пакета (активный узел)
#define SKIP_STEP  0x03 //шибка: доприём до таймаута, затем переход в IDLE
/**
 * Коды ошибок драйвера UART
 */
#define eUART  0x01  //ошибока UART
#define eSEND  0x02  //ошибока передачи бита
#define eLEN   0x04  //ошибока размера пакета
#define eCRC   0x08  //ошибока CRC
#define eRxD   0x10  //ошибока переполнения буфера приема RxD 
#define eTxD   0x20  //ошибока передачи пакета
#define ePDO   0x40  //ошибока переполнения буфера PDO
#define eARBIT 0x80  //проигран арбитраж

/*
 * STATE MACHINE
 */
//void engineState(void);
inline static void initTMR0(void);
inline static void initUART(void);
inline static void init_const(void);
inline static void initINT(void);

inline static void initNewCycle(void);
inline static void actStepIdle(void);

static void sendReadyPack(void);
inline static void setStateSend(void);
inline static void sendStart(void);
static uint8_t getNextByte(void);
static void setBusLevel(void);

static void setError(uint8_t err_mask);

typedef union {
    uint8_t raw[6];

    struct {
        uint8_t upSendUART;
        uint8_t dnSendUART;
        uint8_t checUART; //<маска выбоки бита 
        uint8_t crcPoly; //<CRC полином
        uint8_t loadTMR0; //<величина защитного интервала GAP
    };
} uCAN_const_t;

typedef union {
    uint8_t allFlags;

    struct {
        uint8_t cntRetrySend : 3; //счетчик повторных попыток отправки
        uint8_t echoTX : 1; // режим эхо приема
        uint8_t rcvIsFull : 1; //пакет принят полностью
        uint8_t sndIsFull : 1; //пакет готов к отправке 
        uint8_t sndBit : 1; // отправляемый бит
        uint8_t rcvBit : 1; // принятый бит 

    };
} uCAN_state_t;

typedef struct {
    //пакетные счетчики бит, байт, контроля размера пакета
    uint8_t actStep;
    uint8_t sndByte; //<передаваемый байт
    uint8_t rcvByte; //<принимаемый байт
    uint8_t crcByte; //<байт контрольной суммы
    uint8_t lenBitPack; //длина пакета в битах
    uint8_t bitMask; //<маска позиции бита в байте
    uint8_t cntBitPack; //<счетчик/номер принятого бита в паккете 0-80
    volatile uint8_t *ptrByteSend; //указатель на посылаемый байт пакета
    volatile uint8_t *ptrByteRecv; //указатель на принимаемый байт пакета
} uCAN_control_t;

#endif	/* UART_CORE_H */

