/*
 * lcd.h
 *
 *  Created on: Jan 5, 2018
 *      Author: embedded
 */

#ifndef LCD_H_
#define LCD_H_

#include "general.h"
#include "lpc2xxx.h"

//-------- adresy pinów ekranu LCD ------
#define rs 0x01000000
#define rw 0x00400000
#define en 0x02000000
#define dat 0x00ff0000
#define b_light 0x40000000

void delay37us(void);
void initLCD(void);
void writeLCD (tU8 reg, tU8 znak);
void setLCD();
void LCD_WriteFunction (unsigned int function);
void LCD_WriteNumber (unsigned int number);
void LCD_WriteString ( char * string);
void LCD_WriteCHARINT (unsigned int charint);
void lcdBacklight(tU8 onOff);

#endif /* LCD_H_ */
