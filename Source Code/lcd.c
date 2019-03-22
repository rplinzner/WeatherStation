/*
 * lcd.c
 *
 *  Created on: Jan 5, 2018
 *      Author: embedded
 */
#include "lcd.h"

void delay37us(void) {
    volatile tU32 i;

//Temp test for 140 uS delay
    for (i = 0; i < 6 * 2500; i++)
            asm volatile (" nop");
}

void initLCD(void) {
    IODIR1 |= (dat | en | rs);
    IOCLR1 = (dat | en | rs);

    IODIR0 |= rw;
    IOCLR0 = rw;

    IODIR0 |= b_light;
    IOCLR0 = b_light;

}

void writeLCD(tU8 reg, tU8 znak) {
    volatile tU8 i;

    if (reg == 0)
        IOCLR1 = rs;
    else
        IOSET1 = rs;

    IOCLR0 = rw;
    IOCLR1 = dat;
    IOSET1 = ((tU32) znak << 16) & dat;

    IOSET1 = en;

    for (i = 0; i < 16; i++)
            asm volatile (" nop"); //delay 15 ns x 16 = about 250 ns delay
    IOCLR1 = en;
    for (i = 0; i < 16; i++)
            asm volatile (" nop"); //delay 15 ns x 16 = about 250 ns delay
}

void setLCD() {
    writeLCD(0, 0x30);
    delay37us();
    delay37us();

    //function set
    writeLCD(0, 0x38);
    delay37us();

    //display off
    writeLCD(0, 0x08);
    delay37us();

    //display clear
    writeLCD(0, 0x01);
    delay37us(); //actually only 1.52 mS needed

    //display control set
    writeLCD(0, 0x06);
    delay37us();

    //display control set
    writeLCD(0, 0x0c);
    delay37us();
    //cursor home
    writeLCD(0, 0x02);
    delay37us();
}

void LCD_WriteFunction(unsigned int function) {
    delay37us();
    writeLCD(0, function);
}

void LCD_WriteNumber(unsigned int number) {
    delay37us();
    writeLCD(1, number + '0');
}

void LCD_WriteString(char *string) {
    int c = 0;
    delay37us();
    while (string[c] != '\0') {
        writeLCD(1, string[c]);
        c++;
        delay37us();
    }
}

void LCD_WriteCHARINT(unsigned int charint) {
    delay37us();
    writeLCD(1, charint);
}

void lcdBacklight(tU8 onOff) {
    if (onOff == TRUE)
        IOSET0 = b_light;
    else
        IOCLR0 = b_light;
}
