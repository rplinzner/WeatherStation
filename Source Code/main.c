#include "general.h"
#include "osapi.h"
#include <lpc2xxx.h>
#include <printf_P.h>
#include <printf_init.h>
#include <consol.h>
#include <config.h>
#include "irq/irq_handler.h"
#include "timer.h"
#include "VIC.h"
#include "Common_Def.h"
#include "i2c.h"
#include "eeprom.h"
#include "lcd.h"
#include "DHT11.h"
#include "delays.h"

void RTC_init() {
    CCR = 0x00000012; //
    CCR = 0x00000010;
    ILR = 0x00000000;
    CIIR = 0x00000000;
    AMR = 0x00000000;

    delay_us(500000);
    SEC = 0;    //licznik sekund
    MINx = 25;    //rejestr minut
    HOUR = 15;    //rejestr godzin
    delay_us(500000);
    CCR = 0x00000011;
    delay_us(500000);
}

static void init_irq(tU32 miliseconds) {
    //Zainicjuj VIC dla przerwań od Timera #1
    VICIntSelect &= ~TIMER_0_IRQ;           //Przerwanie od Timera #1 przypisane do IRQ (nie do FIQ)
    VICVectAddr5 = (tU32) IRQ_Test;         //adres procedury przerwania
    VICVectCntl5 = VIC_ENABLE_SLOT | TIMER_0_IRQ_NO;
    //Przypisanie i odblokowanie slotu w VIC
    VICIntEnable = TIMER_0_IRQ;            //Zezwolenie na przerwania od Timera #1

    T0TCR = TIMER_RESET;                    //Zatrzymaj i zresetuj
    T0PR = 0;                              //Preskaler nieużywany
    T0MR0 = ((tU64) miliseconds) * ((tU64) PERIPHERAL_CLOCK) / 1000;
    T0IR = 0xff;                           //Resetowanie flag przerwań
    T0MCR = MR0_I | MR0_R;                  //Generuj okresowe przerwania
    T0TCR = TIMER_RUN;                      //Uruchom timer
}
//-----------------------------------------------------------------------------------    MAIN    ---------------------
int main(void) {
    unsigned int switchStatus;
    tU8 temp_humid_data[5];
    int mode = 1;
    int cl_scr = 0;
    RTC_init();
    initLCD();
    lcdBacklight(TRUE);

    setLCD();
    PINSEL0 &= 0x00000000;
    IODIR1 |= (1 << 14);

    tU8 data[3]; // bufor do przekazywania temperatury przez LM75
    i2cInit();   // zainicjowanie magistrali i2c

    init_irq(10);
    while (1) {

        if (1 == lm75Read(0x90, &data[0], 3)) {
            temp = (((tU16) data[0] << 8) + (tU16) data[1]) >> 7;
        } else {
            temp = 1;
        }

        if (cl_scr == 1) {
            LCD_WriteFunction(0x01);
            cl_scr = 0;
        }

        if (mode == 1) //ekran z czasem
        {
            LCD_WriteFunction(0x80);
            LCD_WriteString("Time:");
            LCD_WriteFunction(0x80 | 0x40);
            LCD_WriteNumber(HOUR / 10);
            LCD_WriteNumber(HOUR % 10);
            LCD_WriteString(":");
            LCD_WriteNumber(MINx / 10);
            LCD_WriteNumber(MINx % 10);
            LCD_WriteString(":");
            LCD_WriteNumber(SEC / 10);
            LCD_WriteNumber(SEC % 10);

            delay_us(1);

        }

        if (mode == 2) //ekran z temperatura
        {
            dht11_request();
            dht11_response();
            for (int i = 0; i < 5; i++) {
                temp_humid_data[i] = dht11_data();
            }

            if ((temp_humid_data[0] + temp_humid_data[1] + temp_humid_data[2] + temp_humid_data[3]) ==
                temp_humid_data[4]) {
                LCD_WriteFunction(0x80);
                LCD_WriteString("Temp OUT: ");
                LCD_WriteCHARINT((((temp_humid_data[2] % 1000) % 100) / 10 + 48));
                LCD_WriteCHARINT((((temp_humid_data[2] % 1000) % 100) % 10 + 48));
                LCD_WriteString(".");
                LCD_WriteNumber(temp_humid_data[3]);
                LCD_WriteCHARINT(223);
                LCD_WriteString("C");
            }

            LCD_WriteFunction(0x80 | 0x40);
            LCD_WriteString("Temp IN:  ");
            LCD_WriteNumber((temp / 2) / 10);
            LCD_WriteNumber((temp / 2) % 10);
            LCD_WriteString(".");
            LCD_WriteNumber((temp & 1) * 5);
            LCD_WriteCHARINT(223); //stopien
            LCD_WriteString("C");

            delay_sec(1);
            delay_ms(50);
        }

        if (mode == 3) //ekran z temperatura
        {
            LCD_WriteFunction(0x80);
            LCD_WriteString("Humidity:");
            LCD_WriteFunction(0x80 | 0x40);

            dht11_request();
            dht11_response();

            for (int i = 0; i < 5; i++) {
                temp_humid_data[i] = dht11_data();
            }

            if ((temp_humid_data[0] + temp_humid_data[1] + temp_humid_data[2] + temp_humid_data[3]) ==
                temp_humid_data[4]) {

                LCD_WriteCHARINT((((temp_humid_data[0] % 1000) % 100) / 10 + 48));
                LCD_WriteCHARINT((((temp_humid_data[0] % 1000) % 100) % 10 + 48));
                LCD_WriteString("%");

            }

            delay_sec(1);
            delay_ms(50);
        }

        switchStatus = (IOPIN0 >> 14) & 0x01;

        if (switchStatus == 0) {
            if (mode < 3) mode++;
            else mode = 1;

            if (mode == 1) {
                LCD_WriteFunction(0x01);
                LCD_WriteString("Time:");
            }

            if (mode == 2) {
                LCD_WriteFunction(0x01);
                LCD_WriteString("Temp");
            }
            if (mode == 3) {
                LCD_WriteFunction(0x01);
                LCD_WriteString("Humidity:");
            }

            delay_sec(1);
        }
    }
    return 0;

}

