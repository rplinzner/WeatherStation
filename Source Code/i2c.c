
#include "general.h"
#include "i2c.h"
#include <lpc2xxx.h>

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define I2C_CONSET (*((volatile unsigned char *) 0xE001C000))
#define I2C_STAT   (*((volatile unsigned char *) 0xE001C004))
#define I2C_DATA   (*((volatile unsigned char *) 0xE001C008))
#define I2C_ADDR   (*((volatile unsigned char *) 0xE001C00C))
#define I2C_SCLH   (*((volatile unsigned short*) 0xE001C010))
#define I2C_SCLL   (*((volatile unsigned short*) 0xE001C014))
#define I2C_CONCLR (*((volatile unsigned char *) 0xE001C018))

#define I2C_REG_CONSET      0x00000040 // Control Set Register        
#define I2C_REG_CONSET_MASK 0x0000007C // Used bits                   
#define I2C_REG_DATA        0x00000000 // Rejestr danych
#define I2C_REG_DATA_MASK   0x000000FF // Used bits                 
#define I2C_REG_ADDR        0x00000000 // Rejestr adresu urzadzenia slave
#define I2C_REG_ADDR_MASK   0x000000FF // Used bits                   
#define I2C_REG_SCLH        0x00000040 // SCL Duty Cycle high register 
#define I2C_REG_SCLH_MASK   0x0000FFFF // Used bits                   
#define I2C_REG_SCLL        0x00000040 // SCL Duty Cycle low register  
#define I2C_REG_SCLL_MASK   0x0000FFFF // Used bits                    

/******************************************************************************
 *
 * Opis:
 *    Sprawdzenie statusu transmisji przez i2c.
 *
 *  Zwraca:
 *      00h Blad magistrali
 *      08h START komunikat przeslany
 *      10h Repeated START condition przeslany
 *      18h SLA + W przeslany, ACK otrzymany
 *      20h SLA + W przeslany, ACK nie otrzymany
 *      28h Bajt danych przeslany, ACK otrzymany
 *      30h Bajt danych przeslany, ACK nie otrzymany
 *      38h Arbitration lost
 *      40h SLA + R przeslany, ACK otrzymany
 *      48h SLA + R przeslany, ACK nie otrzymany
 *      50h Bajt danych otrzymany w master mode, ACK przeslany
 *      58h Bajt danych otrzymany w master mode, ACK nie przeslany
 *      60h SLA + W otrzymany, ACK przeslany
 *      68h Arbitration lost, SLA + W otrzymany, ACK przeslany
 *      70h General call address otrzymany, ACK przeslany
 *      78h Arbitration lost, general call addr otrzymany, ACK przeslany
 *      80h Bajt danych otrzymany with own SLA, ACK przeslany
 *      88h Bajt danych otrzymany with own SLA, ACK nie przeslany
 *      90h Bajt danych otrzymany after general call, ACK przeslany
 *      98h Bajt danych otrzymany after general call, ACK nie przeslany
 *      A0h STOP or repeated START condition otrzymany w trybie slave
 *      A8h SLA + R otrzymany, ACK przeslany
 *      B0h Arbitration lost, SLA + R otrzymany, ACK przeslany
 *      B8h Bajt danych przeslany w trybie slave, ACK otrzymany
 *      C0h Bajt danych przeslany w trybie slave, ACK nie otrzymany
 *      C8h Ostatni bajt przeslany w trybie slave, ACK otrzymany
 *      F8h No relevant status information, SI=0
 *      FFh BLAD
 *
 *****************************************************************************/
tU8 i2cCheckStatus(void) {
    tU8 status = 0;

    while ((I2C_CONSET & 0x08) == 0)   // dopoki flaga przerwan == 0, oczekiwanie na zmiane statusu transmisji
    { ;
    }

    status = I2C_STAT; //odczytanie statusu i wpisanie do zmiennej (flaga nie jest czyszczona tutaj!)

    return status;
}

/******************************************************************************
 *
 * Opis:
 *    Zainicjowanie magistrali i2c.
 *
 *****************************************************************************/
void i2cInit(void) {
    PINSEL0 |= 0x50;

    I2C_CONCLR = 0x6c;    //czyszczenie flag

    I2C_SCLL = (I2C_SCLL & ~I2C_REG_SCLL_MASK) | I2C_REG_SCLL; //reset rejestrow
    I2C_SCLH = (I2C_SCLH & ~I2C_REG_SCLH_MASK) | I2C_REG_SCLH;
    I2C_ADDR = (I2C_ADDR & ~I2C_REG_ADDR_MASK) | I2C_REG_ADDR;
    I2C_CONSET = (I2C_CONSET & ~I2C_REG_CONSET_MASK) | I2C_REG_CONSET;
}

/******************************************************************************
 *
 * Opis:
 *    Generuje komunikat startu kiedy I2C jest wolna.
 *
 * Zwraca:
 *    I2C_CODE_OK lun kod statusu i2c.
 *
 *****************************************************************************/
tS8
i2cStart(void) {
    tU8 status = 0;
    tS8 retCode = 0;

    I2C_CONSET |= 0x20; // ustawienie flagi startu na 1

    while (1)   // czekaj dopoki wyslany zostanie komunikat startu
    {
        status = i2cCheckStatus(); //odczytanie obecnego statusu i2c

        if ((status == 0x08) || (status == 0x10)) //czy komunikat start zostal wyslany
        {
            retCode = I2C_CODE_OK;
            break;
        } else if (status != 0xf8) //BLAD
        {
            retCode = (tS8) status;
            break;
        } else {
            I2C_CONCLR = 0x08; //czyszczenie flagi SI w przypadku gdy odczytany status nie odpowiada zadnemu z warunkow
        }
    }

    I2C_CONCLR = 0x20; //czyszczenie flagi startu

    return retCode;
}

/******************************************************************************
 *
 * Opis:
 *    Generuje komunikat startu gdy i2c jest wolne
 *
 * Zwraca:
 *    I2C_CODE_OK lub kod statusu i2c
 *
 *****************************************************************************/
tS8 i2cRepeatStart(void) {
    tU8 status = 0;
    tS8 retCode = 0;

    I2C_CONSET |= 0x20; // ustawienie flagi startu na 1
    I2C_CONCLR = 0x08;  // wyczyszczenie flagi przerwan

    while (1) // czekaj dopoki wyslany zostanie komunikat startu
    {
        status = i2cCheckStatus(); //pobranie aktualnego statusu i2c

        if ((status == 0x08) || (status == 0x10)) //sprawdzenie czy status ustawiony na start
        {
            retCode = I2C_CODE_OK;
            break;
        } else if (status != 0xf8) //BLAD
        {
            retCode = (tS8) status;
            break;
        } else {
            I2C_CONCLR = 0x08; // wyczyszczenie flagi przerwan
        }
    }

    I2C_CONCLR = 0x20; //wyczyszczenie flagi startu

    return retCode;
}

/******************************************************************************
 *
 * Opis:
 *    Generuje stop
 *
 * Zwraca:
 *    I2C_CODE_OK
 *
 *****************************************************************************/
tS8 i2cStop(void) {
    I2C_CONSET |= 0x10; // STO = 1, ustawienie flagi stop
    I2C_CONCLR = 0x08;  // czysczenie flagi SI

    while ((I2C_CONSET & 0x10) == 0x10) //oczekiwanie do wykrycia komunikatu stop
    { ;
    }

    return I2C_CODE_OK;
}

/******************************************************************************
 *
 * Opis:
 *    Wyslanie znaku przez magistrale 
 *
 * Parametry:
 *     data - znak ktory chcemy wyslac
 *
 * Zwraca:
 *    I2C_CODE_OK   - powodzenie wyslania znaku
 *    I2C_CODE_BUSY - rejestr danych nie jest gotowy
 *
 *****************************************************************************/
tS8 i2cPutChar(tU8 data) {
    tS8 retCode = 0;

    if ((I2C_CONSET & 0x08) != 0)  //sprawdzenie czy SI = 1 (czy mozna uzyskac dostep do rejestru danych)
    {
        I2C_DATA = data;            //wyslane znaku
        I2C_CONCLR = 0x08;            //czysczenie flagi SI
        retCode = I2C_CODE_OK;   //powodzenie wyslania
    } else {
        retCode = I2C_CODE_BUSY;    //gdy rejestr danych jest zajety
    }

    return retCode;
}

/******************************************************************************
 *
 * Opis:
 *    Odczytanie znaku z rejestru danych
 *
 * Parametry:
 *     mode  - I2C_MODE_ACK0 ustawia ACK=0. Slave wysyla kolejny bajt
 *             I2C_MODE_ACK1 ustawia ACK=1. Slave wysyla ostatni bajt
 *             I2C_MODE_READ tryb odczytu danych z rejestru danych
 *     pData - wskaznik na zmienna gdzie zapisujemy odczytane dane
 *
 * Zwraca:
 *    I2C_CODE_OK    - powodzenie
 *    I2C_CODE_EMPTY - brak danych w rejestrze
 *
 *****************************************************************************/
tS8 i2cGetChar(tU8 mode, tU8 *pData) {
    tS8 retCode = I2C_CODE_OK;

    if (mode == I2C_MODE_ACK0) {
        //tryb master zmieniony z transmisji na odbiór

        I2C_CONSET |= 0x04; //ustawienie ACK=0 (informacja dla slave aby ten wyslal kolejny bajt)
        I2C_CONCLR = 0x08;  //czyszczenie flagi przerwan
    } else if (mode == I2C_MODE_ACK1) {
        I2C_CONCLR = 0x04; // ustawia ACK=1 (informacja dla slave aby wyslal ostatni bajt danych)
        I2C_CONCLR = 0x08;
    } else if (mode == I2C_MODE_READ) //sprawdzenie czy tryb master jest ustawiony na odczyt
    {
        if ((I2C_CONSET & 0x08) != 0)  // sprawdzenie czy rejestr danych jest dostepny
        {
            *pData = (tU8) I2C_DATA;  //odczytanie znaku z rejestru danych, przepisanie do zmiennej
        } else {
            retCode = I2C_CODE_EMPTY; //brak dostepnych danych do odczytu
        }
    }

    return retCode;
}

/******************************************************************************
 *
 * Opis:
 *    Wysylanie danych na magistrale i2c
 *
 * Parametry:
 *     addr  - adres urzadzenia
 *     pData - dane do transmisji
 *     len   - ilosc bajtow do wyslania
 *
 * Zwraca:
 *    I2C_CODE_OK    - powodzenie
 *    I2C_CODE_ERROR - BLAD
 *
 *****************************************************************************/
tS8 i2cWrite(tU8 addr, tU8 *pData, tU16 len) {
    tS8 retCode = 0;
    tU8 status = 0;
    tU8 i = 0;

    retCode = i2cStart(); // generowanie komunikatu start

    // transmisja adresu
    if (retCode == I2C_CODE_OK) {
        /* write SLA+W */
        retCode = i2cPutChar(addr);

        while (retCode == I2C_CODE_BUSY) //dopoki rejestr danych zajety
        {
            retCode = i2cPutChar(addr);
        }
    }

    if (retCode == I2C_CODE_OK) {
        // oczekuj dopoki wyslany zostanie adres
        for (i = 0; i < len; i++) {
            // oczekuj dopoki wyslane zostana dane
            while (1) {
                status = i2cCheckStatus(); // odczyt aktualnego statusu i2c

                // SLA+W wyslane i bit ACK otrzymany lub bajt wyslany i bit ACK otrzymany

                if ((status == 0x18) || (status == 0x28)) {
                    // Dane wyslane i otrzymano bit ACK

                    retCode = i2cPutChar(*pData); // zapisuj dane

                    while (retCode == I2C_CODE_BUSY) {
                        retCode = i2cPutChar(*pData);
                    }
                    pData++;

                    break;
                } else if (status != 0xf8) //gdy status nie jest odpowiedni BLAD
                {
                    i = len;
                    retCode = I2C_CODE_ERROR;
                    break;
                }
            }
        }
    }

    // oczekuj dopoki wyslane zostana dane
    while (1) {
        status = i2cCheckStatus(); //odczytaj aktualny status i2c

        /*
         * SLA+W wyslany, ACK otrzymany lub
         * bajt danych wyslany, ACK otrzymany
         */

        if ((status == 0x18) || (status == 0x28)) {
            // dane wyslane i ACK otrzymane
            break;
        } else if (status != 0xf8) // BLAD - brak odpowiedniego statusu i2c
        {
            i = len;
            retCode = I2C_CODE_ERROR;
            break;
        }
    }

    i2cStop(); // generowanie komunikatu stop

    return retCode;
}

tS8
i2cWaitTransmit(void) {
    tU8 status = 0;

    while (1) // oczekuj dopoki dane zostana wyslane
    {
        status = i2cCheckStatus(); // odczytaj aktualny status

        /*
         * SLA+W wyslany, ACK otrzymane lub
         * bajt danych wyslany, ACK otrzymany
         */
        if ((status == 0x18) || (status == 0x28)) {
            return I2C_CODE_OK; // dane wyslane i bit ACK otrzymany
        } else if (status != 0xf8) // BLAD, brak odpowiedniego kodu statusu
        {
            return I2C_CODE_ERROR;
        }
    }
}

tS8
i2cWriteWithWait(tU8 data) {
    tS8 retCode = 0;

    retCode = i2cPutChar(data);
    while (retCode == I2C_CODE_BUSY) {
        retCode = i2cPutChar(data);
    }

    if (retCode == I2C_CODE_OK)
        retCode = i2cWaitTransmit();

    return retCode;
}

tS8 i2cMyWrite(tU8 addr, tU8 *pData, tU16 len) {
    tS8 retCode = 0;
    tU8 i = 0;

    do {
        // generuj komunikat start
        retCode = i2cStart();
        if (retCode != I2C_CODE_OK)
            break;

        /* write address */
        retCode = i2cWriteWithWait(addr);
        if (retCode != I2C_CODE_OK)
            break;

        for (i = 0; i < len; i++) {
            retCode = i2cWriteWithWait(*pData);
            if (retCode != I2C_CODE_OK)
                break;

            pData++;
        }

    } while (0);

    /* generate Stop condition */
    i2cStop();


    return retCode;
}

/******************************************************************************
 *
 * Opis:
 *    Read a specified number of bytes from the I2C network.
 *
 * Params:
 *    [in] addr - adres
 *    [in] pBuf - bufor do odbioru danych
 *    [in] len  - ilosc bajtow do odbioru
 *
 * Zwraca:
 *    I2C_CODE_OK lub I2C status code
 *
 *****************************************************************************/
tS8 i2cRead(tU8 addr, tU8 *pBuf, tU16 len) {
    tS8 retCode = 0;
    tU8 status = 0;
    tU8 i = 0;

    retCode = i2cStart(); //generowanie sygnalu start

    // wyslanie adresu
    if (retCode == I2C_CODE_OK) {
        /* write SLA+R */
        retCode = i2cPutChar(addr);
        while (retCode == I2C_CODE_BUSY) {
            retCode = i2cPutChar(addr);
        }
    }

    if (retCode == I2C_CODE_OK) {

        //oczekuj dopoki wyslany zostanie adres i odbieraj dane
        for (i = 1; i <= len; i++) {
            // oczekuj az wyslane zostana dane
            while (1) {
                status = i2cCheckStatus(); // odczytaj status i2c

                /*
                 * SLA+R wyslany, ACK otrzymany lub
                 * SLA+R wyslany, ACK nie otrzymany lub
                 * bajt danych otrzymany w trybie master, ACK wyslany
                 */

                if ((status == 0x40) || (status == 0x48) || (status == 0x50)) {
                    // dane otrzymane

                    if (i == len) {
                        retCode = i2cGetChar(I2C_MODE_ACK1, pBuf); // generowanie bitu NACK
                    } else {
                        retCode = i2cGetChar(I2C_MODE_ACK0, pBuf);
                    }

                    retCode = i2cGetChar(I2C_MODE_READ, pBuf); //tryb odczytywania danych
                    while (retCode == I2C_CODE_EMPTY) {
                        retCode = i2cGetChar(I2C_MODE_READ, pBuf);
                    }
                    pBuf++;

                    break;
                } else if (status != 0xf8) // BLAD - brak odpowiedniego statusu i2c
                {
                    i = len;
                    retCode = I2C_CODE_ERROR;
                    break;
                }
            }
        }
    }

    i2cStop(); //zatrzymanie transmisji

    return retCode;
}



