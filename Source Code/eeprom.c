#include "general.h"
#include "i2c.h"


#define EEPROM_SIZE       0x0100
#define LOCAL_EEPROM_ADDR 0x0
#define EEPROM_ADDR       0xA0

#define I2C_EEPROM_ADDR   (EEPROM_ADDR + (LOCAL_EEPROM_ADDR << 1)) //adres pamieci eeprom potrzebny do komunikacji przez i2c
#define I2C_EEPROM_RCV    (EEPROM_ADDR + (LOCAL_EEPROM_ADDR << 1) + 0x01) // flaga transimisji - odbierz dane
#define I2C_EEPROM_SND    (EEPROM_ADDR + (LOCAL_EEPROM_ADDR << 1) + 0x00) // flaga transmisji - wyslij dane

//-------------------------------------------------------------------------- FUNKCJE -----------------------------------------------------------------------------------

/******************************************************************************
 *
 * Opis:
 *    Oczekiwanie na zwrot ACK przez i2c
 *
 * Zwraca:
 *    I2C_CODE_OK lub I2C_CODE_ERROR
 *
 *****************************************************************************/
tS8 eepromPoll(void) {
    tS8 retCode = I2C_CODE_OK;
    tU8 status = 0;
    tBool burnEnd = FALSE;

    while (burnEnd == FALSE) {
        retCode = i2cStart(); //generuj sygnal start

        /* Wyslij SLA+W */
        if (retCode == I2C_CODE_OK) {
            /* Zapisz SLA+W */
            retCode = i2cPutChar(I2C_EEPROM_SND);
            while (retCode == I2C_CODE_BUSY) {
                retCode = i2cPutChar(I2C_EEPROM_SND);
            }
        }

        if (retCode == I2C_CODE_OK) {
            /* czekaj dopoki SLA+W wyslany */
            status = i2cCheckStatus(); // odczytaj aktualny status i2c

            if (status == 0x18) //jesli dane przeslano z powodzeniem i otrzymano
            {
                burnEnd = TRUE;
            } else if (status == 0x20) {
                // dane wyslane i bit ACK otrzymany
                // wyslij bit startu, start ponownie
                burnEnd = FALSE;
            } else if (status != 0xf8) // BLAD
            {
                retCode = I2C_CODE_ERROR;
                burnEnd = TRUE;
            }
        }

        i2cStop(); // generowanie sygnalu stop
    }

    return retCode;

}

/******************************************************************************
 *
 * Opis:
 *    Rozpocznij odczyt
 *
 * Parametry:
 *    [in] devAddr - adres urzadzenia z ktorego odbywa sie odczyt
 *    [in] address - adres pamieci eeprom
 *
 * Zwraca:
 *    I2C_CODE_OK lub I2C_CODE_ERROR
 *
 *****************************************************************************/
tS8 eepromStartRead(tU8 devAddr, tU16 address) {
    tS8 retCode = 0;
    tU8 status = 0;

    retCode = i2cStart(); // generowanie komunikatu start

    //Wyslij adres
    if (retCode == I2C_CODE_OK) {
        /* Wyslij SLA+W */
        retCode = i2cPutChar(devAddr);
        while (retCode == I2C_CODE_BUSY) {
            retCode = i2cPutChar(devAddr);
        }
    }

    if (retCode == I2C_CODE_OK) {
#if 0
        // oczekuj dopoki dane zostana wyslane
        while(1)
        {
          status = i2cCheckStatus();

          if( (status == 0x18) || (status == 0x28) )
          {
            //Dane wyslane i bit ACK otrzymany


            // Zapisz dane
            retCode = i2cPutChar( (tU8)(address >> 8) );
            while(retCode == I2C_CODE_BUSY)
            {
              retCode = i2cPutChar( (tU8)(address >> 8) );
            }
            break;
          }
          else if(status != 0xf8) // BLAD
          {
            retCode = I2C_CODE_ERROR;
            break;
          }
        }
#endif

        if (retCode == I2C_CODE_OK) {
            // oczekuj dopoki dane zostana wyslane
            while (1) {
                status = i2cCheckStatus(); // odczytaj aktualny status i2c

                if ((status == 0x18) || (status == 0x28)) {
                    // Dane wyslane i bit ACK otrzymany

                    // Zapisz dane
                    retCode = i2cPutChar((tU8) (address & 0xff));
                    while (retCode == I2C_CODE_BUSY) {
                        retCode = i2cPutChar((tU8) (address & 0xff));
                    }

                    break;
                } else if (status != 0xf8) // BLAD
                {
                    retCode = I2C_CODE_ERROR;
                    break;
                }
            }
        }
    }

    // oczekuj dopoki dane zostana wyslane
    while (1) {
        status = i2cCheckStatus(); // odczytaj aktualny status i2c

        if (status == 0x28) {
            // dane wyslane i bit ACK otrzymany
            break;
        } else if (status != 0xf8) // BLAD
        {
            retCode = I2C_CODE_ERROR;
            break;
        }
    }

    retCode = i2cRepeatStart(); // generowanie komunikatu start

    if (retCode == I2C_CODE_OK) //jesli brak bledow to wyslij adres
    {
        /* Wyslij SLA+R */
        retCode = i2cPutChar(devAddr + 0x01);
        while (retCode == I2C_CODE_BUSY) {
            retCode = i2cPutChar(devAddr + 0x01);
        }
    }

    /* Czekaj dopoki SLA+R wyslany */
    while (1) {
        status = i2cCheckStatus(); // odczytaj aktualny status i2c

        if (status == 0x40) {
            // Dane wyslane i bit ACK otrzymany
            break;
        } else if (status != 0xf8) //BLAD
        {
            retCode = I2C_CODE_ERROR;
            break;
        }
    }


    return retCode;
}

//-------------------------------------------------------------
// BUFOR DO ODCZYTU:
// Bajt 00: Adres urzadzenia R/Wn=0 (WRITE)			(tryb zapisu)
////////// Bajt 01: EEPROM Address MSB   (A14..A08) (najbardziej znaczacy bajt adresu pamieci eeprom)
// Bajt 02: EEPROM Address LSB   (A07..A00)			(najmniej znaczacy  bajt adresu pamieci eeprom)
// Bajt 03: Adres urzadzenia R/Wn=1 (READ)			(tryb odczytu)
// Bajt 04 do 67: Buffer for data read from EEPROM  (bufor do odczytu danych z pamieci)
//-------------------------------------------------------------
//  Parametry:
//      pRxData     wskaznik do bufora sluzacego do odbioru danych
//      Length      ilosc bajtow do odbioru
//
//  Zwraca:
//      MA_OK lub kod statusu I2C 
//
//-------------------------------------------------------------
tS8 eepromPageRead(tU16 address, tU8 *pBuf, tU16 len) {
    tS8 retCode = 0;
    tU8 status = 0;
    tU16 i = 0;

    /* Write 4 bytes, see 24C256 Random Read */
    retCode = eepromStartRead(I2C_EEPROM_ADDR, address);


    if (retCode == I2C_CODE_OK) {
        // oczekuj dopoki wyslany zostanie adres i odbieraj dane
        for (i = 1; i <= len; i++) {
            //oczekuj dopoki wyslane zostana wszystkie bajty danych
            while (1) {
                status = i2cCheckStatus(); //odczytaj aktualny status i2c

                if ((status == 0x40) || (status == 0x48) || (status == 0x50)) {
                    // dane otrzymane

                    if (i == len) {
                        //generowanie bitu NACK gdy otrzymano wszystkie bajty
                        retCode = i2cGetChar(I2C_MODE_ACK1, pBuf);
                    } else {
                        retCode = i2cGetChar(I2C_MODE_ACK0, pBuf);
                    }

                    //odczytaj dane
                    retCode = i2cGetChar(I2C_MODE_READ, pBuf);
                    while (retCode == I2C_CODE_EMPTY) {
                        retCode = i2cGetChar(I2C_MODE_READ, pBuf);
                    }
                    pBuf++;

                    break;
                } else if (status != 0xf8) //BLAD
                {
                    i = len;
                    retCode = I2C_CODE_ERROR;
                    break;
                }
            }
        }
    }

    i2cStop(); //generowanie sygnalu stop

    return retCode;

}

tS8 eepromWrite(tU16 addr, tU8 *pData, tU16 len) {
    tS8 retCode = 0;
    tU8 i = 0;

    do {
        retCode = i2cStart(); // wyslanie komunikatu start
        if (retCode != I2C_CODE_OK)
            break;


        // zapisz EEPROM I2C adres */
        retCode = i2cWriteWithWait(I2C_EEPROM_ADDR);
        if (retCode != I2C_CODE_OK)
            break;

#if 0
        // zapis offsetu high w przestrzeni EEPROM
        retCode = i2cWriteWithWait( (tU8)(addr >> 8));
        if(retCode != I2C_CODE_OK)
          break;
#endif

        // zapis offsetu low w przestrzeni EEPROM
        retCode = i2cWriteWithWait((tU8) (addr & 0xFF));
        if (retCode != I2C_CODE_OK)
            break;

        // zapis danych
        for (i = 0; i < len; i++) {
            retCode = i2cWriteWithWait(*pData);
            if (retCode != I2C_CODE_OK)
                break;

            pData++;
        }

    } while (0);

    i2cStop(); //generowanie komunikatu stop

    return retCode;
}

/******************************************************************************
 *
 * Opis:
 *    Odczyt temperatury z czujnika LM75
 *
 * Parametry:
 *    devAddr - adres urzadzenia z ktorego odbywa sie odczyt
 *    pBuf    - bufor do przechowywania danych
 *    len     - ilosc przesylanych bajtow
 *
 * Zwraca:
 *    I2C_CODE_OK lub I2C_CODE_ERROR
 *
 *****************************************************************************/
tS8 lm75Read(tU8 address, tU8 *pBuf, tU16 len) {
    tS8 retCode = 0;
    tU8 status = 0;
    tU16 i = 0;

    retCode = eepromStartRead(address, 0);

    if (retCode == I2C_CODE_OK) {
        for (i = 1; i <= len; i++) // oczekuj az adres zostanie wyslany i odbieraj dane

        {
            // oczekuj dopoki dane zostana wyslane
            while (1) {
                status = i2cCheckStatus(); //sprawdz status i2c

                if ((status == 0x40) || (status == 0x48) || (status == 0x50)) {
                    // jesli dane otrzymane
                    if (i == len) {
                        retCode = i2cGetChar(I2C_MODE_ACK1, pBuf); //jesli wszystkie dane odebrane to wysylamy bit NACK
                    } else {
                        retCode = i2cGetChar(I2C_MODE_ACK0, pBuf); //jesli nie to wysylamy bit ACK
                    }

                    retCode = i2cGetChar(I2C_MODE_READ, pBuf);  //ustawienie trybu odczytu

                    while (retCode == I2C_CODE_EMPTY) {
                        retCode = i2cGetChar(I2C_MODE_READ, pBuf);
                    }
                    pBuf++;

                    break;
                } else if (status != 0xf8) //gdy BLAD
                {
                    i = len;
                    retCode = I2C_CODE_ERROR;
                    break;
                }
            }
        }
    }

    i2cStop(); //generowanie komunikatu stop

    return retCode;

}
