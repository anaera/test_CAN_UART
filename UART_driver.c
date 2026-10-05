/*
 * File:   api_drive.c
 * Author: Chiper
 *
 * Created on 4 сентября 2026 г., 22:49
 */


#include "UART_core.h"
#include "UART_driver.h"

const uCAN_const_t _flash_UART = {UP_SEND_UART, DN_SEND_UART, CHECK_UART, CRC_POLY, SPEED_NUM};
__eeprom uCAN_const_t _epp_UART = {UP_SEND_UART, DN_SEND_UART, CHECK_UART, CRC_POLY, SPEED_NUM};


uint8_t rxUART[11];
uCAN_const_t constUART;
volatile uCAN_control_t coreUART;

uint8_t tmpUART; // отладочная кончтанта


volatile uint8_t *pTxUART; //__at(0x70); // Вот сюда -- в настоящий Common R
volatile uCAN_state_t stateUART;
volatile uint8_t errorUART;


#define SET_GAP() { TMR0 = constUART.loadTMR0; INTCONbits.TMR0IF = 0; }
#define CLR_GAP() 	{TMR0 = 0xFE;}

/**
//------------------------------------------------------------------------------
//TODO это чуть позже
//тактовая 16мгц (шина 4мгц)
// TMR0 - 50 микросекунд (шаг интервала разрешения коллизий доступа к шине)
 */
//500, 250, 125, 62.5, 31.25, 15.1625, 15.1625, 15.1625
// 115.2 34

//const uint8_t time_bit => {2,  4,  8, 16,  32,  64,  64,  64} мкс;
//const uint8_t time_byte => (20, 40,80,160....; //микросекунд
//prg_time 50мкс
//GAP time (time_byte+prg_time)*1.5 => 110,140, 200, 320, 560, 190, 190, 190};
// TMR0 = oxFF - GAP_time/(TMR0_tick=4мкс) = {0xE3, 0xDC, 0xCD, 0xAF, 0x73, oxCF, oxCF, 0xCF};s
const uint8_t tab_load_SPBRGL[8] = {7, 15, 31, 63, 127, 34, 34, 34};
//const uint8_t gap_time [8] = {112, 140, 200, 320, 560, 190, 190, 190};
const uint8_t tab_load_TMR0[8] = {0xE3, 0xDC, 0xCD, 0xAF, 0x73, 0xCF, 0xCF, 0xCF};

inline static void initTMR0(void) {
    //init_TMR0
    // PSA assigned; PS 1:16; TMRSE Increment_hi_lo; mask the nWPUEN and INTEDG bits
    // nWPUEN = 1 INTEDG = 0 TMRCS = 0 TMRSE = x  PSA = 0x03
    OPTION_REG = (uint8_t) ((OPTION_REG & 0x83) | (0x80 | 0x03));
    SET_GAP(); // 0xCE дополнение к 49 до 255

    //OSC = 16 000 000; тактирование Fosc/4 = 16000000/4 = 4 000 000
    //Прескалер 1:4, (Fosc/4 ) / 4 = 4 000 000/16 = 250 000
    //FF-CE = 31(49dec)один тик 250 000 гц -> 4 мкс *49 = 196 (200)мкс (us)
    //FF-90 = 6F (111) один тик 250 000 гц -> 4 мкс 111 = 444 (500)мкс (us)
}
//---------------------------------------------------------------------
// Инициализация UART
//---------------------------------------------------------------------

inline static void initUART(void) {
    // ANSELHbits.ANS11 = 0;
    // настройка пинов
    TRISCbits.TRISC4 = 0; //TxD 
    TRISCbits.TRISC5 = 1; //RxD
    // сброс регистров
    TXSTA = 0;
    RCSTA = 0;
    BAUDCON = 0;

    // настраиваем скорость обмена
    // SYNC=0 BRGH=1 BRG16=1   ->  Fosc/4*[N+1] 
    TXSTAbits.SYNC = 0; //TX Асинхронный режим SYNC
    TXSTAbits.BRGH = 1; //высокоскоростной BRGH
    BAUDCONbits.BRG16 = 1; //BRG16 16-битный генератор скорости
    SPBRGL = tab_load_SPBRGL[SPEED_NUM]; //читаем делитель скорости
    SPBRGH = 0;

    // включение УАРТ
    TXSTAbits.TXEN = 1; //TX Включить TX передачу
    RCSTAbits.CREN = 1; //CREN Включить RX прием
    RCSTAbits.SPEN = 1; //SPEN Включить УАРТ1 
}

inline static void init_const(void) {
    uint8_t cnt;
    for (cnt = 0; cnt < 4; cnt++) {
        constUART.raw[cnt] = _flash_UART.raw[cnt];
    }
    constUART.loadTMR0 = tab_load_TMR0[SPEED_NUM];
}

inline static void initINT(void) {
    //Разрешаем прерывание от Таймера 0
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
#ifdef INT_RA2
    //Разрешаем прерывание от INT0
    OPTION_REGbits.INTEDG = 0; //прерывание по падающему фронту
    INTCONbits.INTF = 0;
    INTCONbits.INTE = 0; //НЕ_Разрешаем прерывание от INT0
#else
    //Разрешаем прерывание от RA3 ао падающему фронту
    IOCANbits.IOCAN3 = 1; //спадающий фронт - ДА
    IOCAPbits.IOCAP3 = 0; //восходящий  - НЕТ

    IOCAFbits.IOCAF3 = 0; //сбрасываем флаг RA3
    INTCONbits.IOCIF = 0; //сбрасываем флаг прерывания
    INTCONbits.IOCIE = 0; //НЕ_Разрешаем прерывание от RA
#endif

    //Разрешение прерывание от приемника UART
    PIE1bits.RCIE = 1;
}
//------------------------------------------------------------------------------
//Драйвер API
//------------------------------------------------------------------------------

void initDriverUART(void) {
    //инициализируем константные параметры драйвера
    init_const();
    stateUART.allFlags = 0;
    //для инициализации рабочих параметров при следующем переходе в режим IDLE
    coreUART.actStep = SKIP_STEP;
    //инициализируем оборудовние драйвера
    initUART();
    initTMR0();
    initINT();

}

uint8_t isRxReady(void) {
    return (stateUART.rcvIsFull == 1);
}

uint8_t isTxReady(void) {
    return (stateUART.sndIsFull == 0);
}

void setEcho(void) {
    stateUART.echoTX = 1;
}

void clrEcho(void) {
    stateUART.echoTX = 0;
}

static void setError(uint8_t err_mask) {
    errorUART = errorUART | err_mask;
}

uint8_t getError(void) {
    return errorUART;
}

void clrError(void) {
    errorUART = 0;
}

volatile uint8_t * recvPack(void) {
    return &rxUART[0];

}

void sendPack(uint8_t *ptrUART) {
    pTxUART = ptrUART;
    stateUART.cntRetrySend = TX_RETRY;
    stateUART.sndIsFull = 1;
}

uint8_t checkSendPack(uint8_t *ptrUART) {
    if (isTxReady()) { //если буфер передачи свободен
        if (!isRxReady()) { //прием передаваемых пакета )
            sendPack(ptrUART);
            return 1;
        }
    }
    return 0;
}

void clrRxReady(void) {
    stateUART.rcvIsFull = 0;
}

uint8_t updByteCRC(uint8_t crc, uint8_t byte) {
    uint8_t i = 1;
    crc = crc ^ byte;
    while (i) {

        //в развернутом виде итоговый код короче
        //     crc = crc & 0x80 ? (uint8_t) (crc << 1) ^ constUART.crcPoly : (uint8_t) (crc << 1);
        if (crc & 0x80) {
            crc = (uint8_t) ((crc << 1) ^ constUART.crcPoly);
        } else {
            crc <<= 1;
        }
        i <<= 1;
    }
    return crc;
}

//------------------------------------------------------------------------------
// Машина состояний
//------------------------------------------------------------------------------
// Драйвер может находится в каждый момент времени в одном из четыырех состояний,
//три актвных - SEND_TEP,RECV_STEP, SKIP_STEP и одном пассивном -IDLE_STEP.
// В актывном состоянии он не может находится более интервала GAP. Если этот
// интервал истекает он безусловно переходит в состоянии IDLE_STEP.
//------------------------------------------------------------------------------
// 1) IDLE_STEP - состояние простоя, ожидания какой либо активности - приема,
// передачи или обработки ошибочного пакета. В этом состоянии драйвер слушает
// шину данных (наличие спадаюющего фронта на пине внешнего прерывания INT) и
// анализирует флаг готовности нового пакета для передачи, при этом:
// 1 - наличие флага готовности нового пакета на передачу вызовет переход в
// режим передачи этого пакета.
// 2 - случилось прерывание по спадающему фронту(start-посылка) переход в режим
// приема (при этом запрещаем дальнейшую реакцию на изменение сигнала на шине -
// запрещаем прерывание INT)
//------------------------------------------------------------------------------
// 2) SEND_STEP - режим передачи в который драйвер переходит только из режима
// простоя, по установленному флагу наличия пакета на передачу. Дальнейшее
// изменение состояния блокируется на время GAP-интервала. 
// В этом состоянии передача пакета осуществляется асинхронно по прерываниям
// модуля UART и при этом:
// 1 - успешное завершение передачи пакета завершается по истечению защитного
// GAP интервала и соответчственно разблокировки машины состояния и переключения
// машины состояний в режим IDLE 
// 2 - любая ошибка при передаче пакета приводит к переходу к состоянию SKIP
// внутри драйвера с последующим прекращением передачи бит данных пакета,
// завершению защитного GAP-интервала и, соответчственно, разблокировки машины
// состояния и переключения машины состояний в режим IDLE 
// 3 - если в процессе передачи проигрывается арбитраж приоритетного доступа к
// шине происходит переход машины состояния из режима передачи SEND_STEP в режим
// приема RECV_STEP внутри драйвера и продолжается прием данных от выигравшего
// арбитраж устройства
//------------------------------------------------------------------------------
// 3) RECV_STEP - режим приема данных с шины в который драйвер переходит из
// состояния IDLE при обнаружении активности шины(прерывание INT) или в случае
// проигранного арбитража режима передачи. В этом состоянии прием пакета
// осуществляется асинхронно по прерываниям модуля UART и
// 1 - успешное завершение приема пакета завершается по истечению защитного
// GAP-интервала, разблокировки машины состояния и переключения машины состояний
// в режим IDLE
// 2 - любая ошибка при приеме пакета приводит к переходу к состоянию SKIP
// внутри драйвера с последующим приемом оставшихся передаваемых по шине бит
// данных пакета, завершению защитного GAP-интервала и, соответчственно,
// разблокировки машины состояния и переключения ее в режим IDLE
//------------------------------------------------------------------------------
// 4) SKIP_STEP - состояние обработки ошибки возникшей при приеме/передачи пакета
// В этом состоянии драйвер дожидается окончания поступления данных с шины приема,
// завершения защитного GAP-интервала, разблокировки машины состояния и 
// переключения ее в режим IDLE.
// При переключении машины состояния в режим IDLE_STEP из любого предшествующего
// состояния влючается прерывание INT и осуществляется контроль активности шины
// данных (спадающего фронта по сигнали START).
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Процедуры для обработчика прерываний - ISR
//------------------------------------------------------------------------------
//Переход в состояние SEND
//------------------------------------------------------------------------------

static void sendReadyPack(void) {
    if (stateUART.sndIsFull) {
        if (stateUART.cntRetrySend) //исчерпано кол-во повторов текущего пакета?
        {
            //	RA1_Toggle();
            setStateSend(); //переход в режим передачи
            sendStart(); //переключились
            return;
        } else {
            stateUART.sndIsFull = 0; //сбрасываем флаг
            setError(eTxD);
        }
    }
}

inline static void setStateSend(void) {
    RA1_SetLow();
    SET_GAP(); //setGapTMR0(); защищаем передачу
#ifdef INT_RA2
    INTCONbits.INTE = 0; //запрещаем прерывание от INT0    
    INTCONbits.INTF = 0; //сбрасываем флаг 

#else

    INTCONbits.IOCIE = 0;
    INTCONbits.IOCIF = 0;
    IOCAFbits.IOCAF3 = 0;

#endif
    coreUART.actStep = SEND_STEP;
}

inline static void sendStart(void) {
    coreUART.ptrByteSend = pTxUART; //устанавливаем указатель на начало буфера передачи
    coreUART.sndByte = getNextByte(); // взять первый байт
    setBusLevel(); //первый пошел (передача)
    stateUART.cntRetrySend--;
    return; //пошла передача
}

static uint8_t getNextByte(void) {
    return *coreUART.ptrByteSend++;
}

static void setBusLevel(void) {
    stateUART.sndBit = 0;
    if (coreUART.sndByte & coreUART.bitMask)stateUART.sndBit = 1;
    TXREG = stateUART.sndBit ? constUART.upSendUART : constUART.dnSendUART;
    //Это отладка
    //tmpUART = stateUART.sndBit ? constUART.upSendUART : constUART.dnSendUART;

}

//------------------------------------------------------------------------------
//Переход в состояние IDLE
//------------------------------------------------------------------------------

void nextStepIDLE(void) {
    RC2_Toggle();
    SET_GAP();
    if (coreUART.actStep == IDLE_STEP) {
        sendReadyPack();
        return;
    }
    initNewCycle();
    actStepIdle();
    RA1_SetHigh();
    RA0_SetHigh();
    coreUART.actStep = IDLE_STEP; //ждем приема или передачи
    return;
}

inline static void initNewCycle(void) {
    coreUART.ptrByteRecv = rxUART; //устанавливаем указатель на начало буфера приема
    coreUART.cntBitPack = 0;
    coreUART.bitMask = 0x80;
    coreUART.cntBitPack = 0;
    //coreUART.lenPack = CAN_LEN_HEADER + 1; //(lenHEAD==2)+(lenCRC==1)
    coreUART.lenBitPack = (BYTE_LEN_HEADER + 1) << 3; //(lenHEAD==2)+(lenCRC==1)
    coreUART.crcByte = 0;
}

inline static void actStepIdle(void) {
    //для контролия линмм RxD включаем прерывание, 
#ifdef INT_RA2
    {
        INTCONbits.INTF = 0; //сбрасываем флаг
        INTCONbits.INTE = 1; //прерывание на прием активно
    }
#else
    {
        INTCONbits.IOCIF = 0;
        IOCAFbits.IOCAF3 = 0;
        INTCONbits.IOCIE = 1;
    }
#endif
}

//------------------------------------------------------------------------------
//Переход в состояние RECV
//------------------------------------------------------------------------------
// в режиме IDLE случилось прерывание по спадающему фронту(start - посылка)
//переход в режим приема (запрещаем прерывание INT0)
//------------------------------------------------------------------------------

void setStateReceive(void) {
    RC1_Toggle();
    SET_GAP(); //на время до полного приема первого байта в RCREG UART
    //за время от start до stop  прием байта в регистк иустановка флага RCIF
    //    if (coreUART.actStep == IDLE_STEP) {
    coreUART.actStep = RECV_STEP;
    RA0_SetLow();
    //    }
}
//------------------------------------------------------------------------------
//Принят следующий байт UART
//------------------------------------------------------------------------------
#define MAX_LEN_DATA 8
#define HEAD_BIT_LEN 16
#define ARBIT_SEQ_LEN 12

void engineUART(void) {
    SET_GAP(); //setGapTMR0(); устанавливаем таймаут ожидания следующего байте
    //Это отладка
    //stateUART.rcvBit = (tmpUART & constUART.checUART) ? 1 : 0; //устанавливаем в msbBit
    // Чтение и обработка принятого бита (принятый байт) (Очищаем буфер приема RCREG!)
    stateUART.rcvBit = (RCREG & constUART.checUART) ? 1 : 0; //устанавливаем в msbBit
    // Проверяем наличие аппаратных ошибок в приёмнике USART и сбрасываем их
    if (RCSTAbits.OERR) { //RCSTAbits.FERR |
        RCSTAbits.CREN = 0;
        NOP();
        RCSTAbits.CREN = 1;
        setError(eUART); //  0x01  ошибока UART
        coreUART.actStep = SKIP_STEP;
        return;
    }
    // Если ранее была какая либо ошибка т.е. SKIP_STEP
    // Сбрасываем все последующие байты принимаемого пакета до  таймаута пакета
    // gapTime - время таймаута,которое считается в TMR0
    if (coreUART.actStep == SKIP_STEP) {
        RC0_Toggle();
        return;
    }
    // Предидущий принятый пакет уже обработан?
    if (stateUART.rcvIsFull) { //Нет
        setError(eRxD); // 0x10  ошибока переполнения буфера приема RxD 
        coreUART.actStep = SKIP_STEP;
        return;
    }
    //Контроль ошибки передачи бита и Арбитраж шины
    if (coreUART.actStep == SEND_STEP) {
        if (stateUART.rcvBit != stateUART.sndBit) {
            // Зона арбитража(Первые12 бит пакета) приоритета пакета (переключения передачи/приёма)
            if (coreUART.cntBitPack < ARBIT_SEQ_LEN) {
                if (stateUART.rcvBit == 0) {
                    // Проигрыш арбитражжа получили ноль когда посылали единицу
                    setError(eARBIT); // // 0x80  проигран арбитраж
                    coreUART.actStep = RECV_STEP;
                    // уступили (переключились из режима передачи в режам приема)
                    //продолжаем собирать пакет победителя
                    stateUART.cntRetrySend++; //передача пакета не состоялась
                    RA0_SetLow();
                    RA1_SetHigh();
                } else {
                    setError(eSEND); // 0x02  ошибока передачи бита
                    coreUART.actStep = SKIP_STEP; // следующей посылки и соответственно приема не будет
                    return; // по окончанию защитного интервала stateEngine переключит в IDLE_STEP
                }
            } else {
                // Мы вышли за пределы арбитражной части пакета 
                setError(eSEND); // 0x02  ошибока передачи бита
                coreUART.actStep = SKIP_STEP; // следующей посылки и соответственно приема не будет
                return; // по окончанию защитного интервала stateEngine переключит в IDLE_STEP
            }
        }
    }
    // Все нормально, можно обрабатывать валидный бит пакета (кладем в текущий байт)
    coreUART.rcvByte <<= 1;
    if (stateUART.rcvBit) {
        coreUART.rcvByte |= 0x01;
    }
    // Обновление контрольной суммы с учетом принятого бита state.flags mask = 0x80
    coreUART.crcByte = ((uint8_t) (coreUART.crcByte << 1) ^ (((coreUART.crcByte ^ ((uint8_t) (stateUART.rcvBit << 7))) & 0x80) ? constUART.crcPoly : 0));
            //                     ((uint8_t) (crc << 1)              ^ (((crc ^ msb_bit) & 0x80) ? STD_POLY : 0));
            // Управление позицией бита внутри байта для приема/передачи следующего бита
            coreUART.bitMask >>= 1; //следующий бит в байте
            coreUART.cntBitPack++; //следующий бит в пакете
    if (!coreUART.bitMask) { // Все биты байта принят/передан?
        coreUART.bitMask = 0x80; //Переход к следующему байту
                // Завершился приём заголовка пакета? Его длина 16 бит - 2 байта.
        if (coreUART.cntBitPack == HEAD_BIT_LEN) { //следующий бит 16 (текущий 15!)
            //coreUART.lenData = coreUART.rcvByte & 0x0F; //Выделяем длину пакета данных
            if ((coreUART.rcvByte & 0x0F) > MAX_LEN_DATA) {
                setError(eLEN); // 0x04  ошибока размера пакета
                        coreUART.actStep = SKIP_STEP;
                return;
            }
            coreUART.lenBitPack += (uint8_t) ((coreUART.rcvByte & 0x0F) << 3); // coreUART.lenPack уже содержит длину заголовка +1(crc)
                    coreUART.lenBitPack--; //от размера массива к номеру последнего элемента
        }
        // Полностью принято нужное количество байтов?
        if (coreUART.cntBitPack > coreUART.lenBitPack) {
            // Сюда заходим только когда == и тут проверяем CRC
            if (coreUART.crcByte == 0) {
                if (coreUART.actStep == RECV_STEP) {
                    stateUART.rcvIsFull = 1;
                } else {
                    if (coreUART.actStep == SEND_STEP) {
                        stateUART.sndIsFull = 0;
                                // Если включён режим эха то добавляем готовность приема
                        if (stateUART.echoTX) {
                            stateUART.rcvIsFull = 1;
                        }
                    }
                }
                //CLR_GAP();
                return;
            } else {
                setError(eCRC); // 0x08  ошибока CRC
                        coreUART.actStep = SKIP_STEP;
                return;
            }
        }
        // Продолжаем прием/передачу пакета данных
        *coreUART.ptrByteRecv = coreUART.rcvByte;
                coreUART.ptrByteRecv++; //  следующий байт 
        if (coreUART.actStep == SEND_STEP) { //  передаем ? 
            coreUART.sndByte = getNextByte(); //да, передаем еще
        }
    }

    if (coreUART.actStep == SEND_STEP) { // Передача очередного бита
        setBusLevel();
                SET_GAP(); //setGapTMR0(); защищаем передачу
    }
}
