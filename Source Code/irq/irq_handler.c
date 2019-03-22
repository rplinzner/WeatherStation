/*************************************************************************************
 *
 * @Description:
 * Przykładowa procedura obsługi przerwania dostosowania do 
 * obsługi przerwań wektoryzowanych.
 * Procedura ta przełącza stan diody P0.9.
 * 
 * 
 * @Authors: Michał Morawski, 
 *           Daniel Arendt, 
 *           Przemysław Ignaciuk,
 *           Marcin Kwapisz
 *
 * @Change log:
 *           2016.12.01: Wersja oryginalna.
 *
 ******************************************************************************/

#include "../general.h"
#include <lpc2xxx.h>
#include "irq_handler.h"
#include "../timer.h"

#include "../Common_Def.h"

int step = 0;


void IRQ_Test(void)
{
	int sk1 = (int)((temp/2)/10);
	int sk2 = (int)((temp/2)%10);

	int t=(sk1*10) + sk2;

	if (t>=28)
	{
	     static tU32 stepmotorSteps[4] = {0x00201000, 0x00200000, 0x00000000, 0x00001000};  //P0.21 and P0.12 are used to contol the stepper motor

	     IODIR0 |= 0x00201000;

	     step++;
	     if(step == 4) step = 0;

		 IOCLR0 = stepmotorSteps[0];
		 IOSET0 = stepmotorSteps[step];
	}

		 T0IR = TIMER_MR0_INT;
		 VICVectAddr = 0x00;
}

