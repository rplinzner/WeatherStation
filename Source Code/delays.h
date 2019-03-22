/*
 * delays.h
 *
 *  Created on: Jan 5, 2018
 *      Author: embedded
 */

#ifndef DELAYS_H_
#define DELAYS_H_

#include "general.h"
#include "osapi.h"
#include <lpc2xxx.h>
#include <printf_P.h>
#include "timer.h"

void delay_us (tU32 microsec);
void delay_ms (tU32 milisec);
void delay_sec (tU32 seconds);

#endif /* DELAYS_H_ */
