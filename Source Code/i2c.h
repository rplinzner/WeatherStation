#ifndef _I2C_H
#define _I2C_H

//tryby
#define I2C_MODE_ACK0 0
#define I2C_MODE_ACK1 1
#define I2C_MODE_READ 2

// zwracane kody 
#define I2C_CODE_OK   1
#define I2C_CODE_DATA 2
#define I2C_CODE_RTR  3

#define I2C_CODE_ERROR -1
#define I2C_CODE_FULL  -2
#define I2C_CODE_EMPTY -3
#define I2C_CODE_BUSY  -4

#define I2C_SLAVEADR_RCV  0xA1
#define I2C_SLAVEADR_SEND 0xA0

// zestaw rejestrow do kontroli I2C

#define I2C_CONSET_I2EN              0x40   // Bit 6: I2EN uruchomienie interfejsu i2c 
#define I2C_CONSET_I2EN_DISABLED     0x00   // wylaczony
#define I2C_CONSET_I2EN_ENABLED      0x01   // wlaczony

#define I2C_CONSET_STA               0x20   // Bit 5: STA flaga startu
#define I2C_CONSET_STA_NO_START      0x00   // brak startu 
#define I2C_CONSET_STA_START         0x01   // start

#define I2C_CONSET_STO               0x10   // Bit 4: STO (flaga stop) 
#define I2C_CONSET_STO_NO_STOP       0x00   // brak stopu 
#define I2C_CONSET_STO_STOP          0x01   // stop

#define I2C_CONSET_SI                0x08   // Bit 3: SI (I2C flaga przerwania) 
#define I2C_CONSET_SI_NO_INTERRUPT   0x00   // brak przerwania
#define I2C_CONSET_SI_INTERRUPT      0x01   // przerwanie

#define I2C_CONSET_AA                0x04   // Bit 2: AA (ACK - bit potwierdzenia odbioru danych)
#define I2C_CONSET_AA_NO_ACK         0x00   /* Not Acknowledge (Not Acknowledge will be sent when data byte is received) */
#define I2C_CONSET_AA_ACK            0x01   /* Acknowledge (Bit Acknowledge will be sent if certain conditions occur) */

// rejestr statusu i2c 

#define I2C_STAT_STATUS              0xff   /* Bit 7-0: STATUS (Status) */

// rejestr danych i2c

#define I2C_DATA_DATA                0xff   /* Bit 7-0: DATA (Dane) */


// I2C rejestr adresu urzadzenia slave 

#define I2C_ADDR_ADDR                0xfe   // Bit 7-1: ADDR - adres urzadzenia slave
#define I2C_ADDR_GC                  0x01   // Bit 0: GC (General call bit) 
#define I2C_ADDR_GC_NOT_GENERAL      0x00   // Not general call 
#define I2C_ADDR_GC_GENERAL          0x01   // General call received 

// I2C SCL Duty Cycle high register 
#define I2C_SCLH_COUNT               0xffff /* Bit 15-0: COUNT (Count for SCL high time period) */

// I2C SCL Duty Cycle Low Register 

#define I2SCLL_COUNT                 0xffff /* Bit 15-0: COUNT (Count for SCL low time period) */

// Czyszczenie rejestrow I2C 

#define I2C_CONCLR_I2ENC             0x40   /* Bit 6: I2ENC (I2C interface disable) */
#define I2C_CONCLR_I2ENC_NO_EFFECT   0x00   /* No effect */
#define I2C_CONCLR_I2ENC_CLEAR       0x01   /* Clear  (I2C Disabled) */

#define I2C_CONCLR_STAC              0x20   /* Bit 5: STAC (Start flag clear) */
#define I2C_CONCLR_STAC_NO_EFFECT    0x00   // brak efektu
#define I2C_CONCLR_STAC_CLEAR        0x01   // wyczysc flage start

#define I2C_CONCLR_SIC               0x08   /* Bit 3: SIC (I2C interrupt clear) */
#define I2C_CONCLR_SIC_NO_EFFECT     0x00   /* No effect */
#define I2C_CONCLR_SIC_CLEAR         0x01   /* Clear interrupt */

#define I2C_CONCLR_AAC               0x04   /* Bit 2: AAC (Assert acknowledge clear) */
#define I2C_CONCLR_AAC_NO_EFFECT     0x00   /* No effect */
#define I2C_CONCLR_AAC_CLEAR         0x01   /* Clear acknowledge */


tU8  i2cCheckStatus(void);
void i2cInit(void);
tS8  i2cStart(void);
tS8  i2cRepeatStart(void);
tS8  i2cStop(void);
tS8  i2cPutChar(tU8 data);
tS8  i2cGetChar(tU8  mode, tU8* pData);
tS8  i2cWrite(tU8  addr, tU8* pData, tU16 len);
tS8  i2cRead(tU8  addr, tU8* pBuf, tU16 len);
tS8  i2cMyWrite(tU8  addr, tU8* pData, tU16 len);
tS8  i2cWriteWithWait(tU8 data);
tS8  i2cWaitTransmit(void);

#endif
