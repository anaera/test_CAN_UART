/* 
 * File:   UART_driver.h
 * Author: Chiper
 *
 * Created on 4 сентября 2026 г., 23:59
 */

#ifndef UART_DRIVER_H
#define	UART_DRIVER_H
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "debug.h"

#define INT_RA2 //Используемое прерывание INT0 или INT_RA3
//#define _TMR0 0x90 //Константа на GAP_TIME 500мкс
//#define _TMR0 0xCE //Константа на GAP_TIME 200мкс

#define BYTE_LEN_HEADER 2 //длина заголовка пакета

#define TX_RETRY 3 //Максимальное число повторов передачи пакета

#define SPEED_NUM 2 // номер входа в таблице скоростей 125000бит/с
#define UP_SEND_UART 0xFC // байт посылка рецесивного бита (LSB для UART)
#define DN_SEND_UART 0x00 // байт посылка доминантного бита
#define CHECK_UART 0x10 //позиция бита выборки
#define CRC_POLY 0x31 //полином CRC

/*
 * API CAN DRIVER
 */
void initDriverUART(void); //инициализация драйвера
uint8_t isRxReady(void); //есть принятый пакет, да-1, нет -0.
uint8_t isTxReady(void); //есть готовность к передаче нового пакета, да-1, нет -0.
void clrRxReady(void); // принятый пакет обработан
uint8_t getError(void); // получить ошибки драйвера
void clrError(void); //сбросить ошибки
void setEcho(void); // установить эхо режим
void clrEcho(void); // отменить эхо режим
volatile uint8_t * recvPack(void); //получить адрес принятого пакета
void sendPack(uint8_t *ptrUART); //передать адрес пакета на отправку      
uint8_t checkSendPack(uint8_t *ptrUART); // защищенная передача адреса пакета на отправку
uint8_t updByteCRC(uint8_t crc, uint8_t byte);
/*
 * Обработчик прерываний - ISR
 */
//переход в режим ожидания по окончании интервала TMR0 и последующий
//переход в режим приема (запрет прерывания INT0) из очередного такта IDLE
void nextStepIDLE(void);
//переход в режим приема (запрет прерывания INT0)
void setStateReceive(void);
// прием(передача) очередного байта UART
void engineUART(void);
#endif	/* UART_DRIVER_H */

