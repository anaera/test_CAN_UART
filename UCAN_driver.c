/*
 * File:   CAN_driver.c
 * Author: Chiper
 *
 * Created on 15 сентября 2026 г., 0:16
 */


#include "UCAN_driver.h"
#include "UART_driver.h"

#pragma udata access fast_can_buffers 
volatile packCAN_t txUCAN;
#pragma udata access common_flags 
volatile uint8_t *pRxUART;
#pragma udata

extern uint8_t tmpUART;
uint8_t node_owner;

void initDriverUCAN(void) {
    node_owner = 0x0A;
}

void packSendSelect(void) {
    txUCAN.cmd_can = 0x07;
    txUCAN.lenData = 1;
    txUCAN.data[0] = tmpUART;
    tmpUART++;
}

void sendPackUCAN(void) {
    uint8_t cnt = 0;
    uint8_t crc_pos;
    uint8_t crc_val = 0;
    if (isTxReady()) { //если буфер передачи свободен
        if (!isRxReady()) { //прием передаваемых пакета )
            txUCAN.canHead = 0; //пытаемся сформировать пакет данных
            //txCAN.rtr = 0; //пока это не пакет ответа на запрос
            packSendSelect(); //выбираем приоритетный пакет из ожидающих 
            // отправляем пакеты HBT, PDO, SDO, у всех длина <> 0!
            if (txUCAN.lenData != 0) { // если были какието данные то отправляем пакет
                txUCAN.hiNode = node_owner >> 3; //можно вставить номер ноды
                txUCAN.loNode = node_owner & 0x07;
                crc_pos = txUCAN.lenData + BYTE_LEN_HEADER;
                for (cnt = 0; cnt < crc_pos; cnt++) { //посчитать CRC и  вставить 
                    crc_val = updByteCRC(crc_val, txUCAN.raw[cnt]);
                }
                txUCAN.raw[crc_pos] = crc_val;
                sendPack((uint8_t *) & txUCAN.raw);
            }
        }
    }
}

void recvPackUCAN(void) {
    uint8_t error;
    if (isRxReady()) {
        error = getError();
		clrError();
        pRxUART = recvPack();
        //тут забрать/обработать данные принятого пакета
        tmpUART = (((packCAN_t *) pRxUART)->raw[2]) + 1;
        clrRxReady();

    }
}