/*
 * delays.c
 *
 *  Created on: Jan 5, 2018
 *      Author: embedded
 */

#include "delays.h"

void delay_sec(tU32 seconds) {
    T1TCR = TIMER_RESET;                    //Zatrzymaj i zresetuj
    T1PR = PERIPHERAL_CLOCK - 1;             //jednostka w preskalerze
    T1MR0 = seconds;
    T1IR = TIMER_ALL_INT;                  //Resetowanie flag przerwań
    T1MCR = MR0_S;                          //Licz do wartości w MR0 i zatrzymaj się
    T1TCR = TIMER_RUN;                      //Uruchom timer

    // sprawdź czy timer działa
    // nie ma wpisanego ogranicznika liczby pętli, ze względu na charakter procedury
    while (T1TCR & TIMER_RUN) {
    }
}

void delay_ms(tU32 milisec) {
    T1TCR = TIMER_RESET;                    //Zatrzymaj i zresetuj
    T1PR = 0;                              //Preskaler nieużywany
    T1MR0 = ((tU64) milisec) * ((tU64) PERIPHERAL_CLOCK) / 1000;
    T1IR = 0xff;                           //Resetowanie flag przerwań
    T1MCR = MR0_S;                          //Generuj okresowe przerwania
    T1TCR = TIMER_RUN;

    while (T1TCR & TIMER_RUN) {
    }
}

void delay_us(tU32 microsec) {
    T1TCR = TIMER_RESET;                    //Zatrzymaj i zresetuj
    T1PR = 0;                              //Preskaler nieużywany
    T1MR0 = ((tU64) microsec) * ((tU64) PERIPHERAL_CLOCK) / 1000000;
    T1IR = 0xff;                           //Resetowanie flag przerwań
    T1MCR = MR0_S;                  //Generuj okresowe przerwania
    T1TCR = TIMER_RUN;

    while (T1TCR & TIMER_RUN) {
    }
}
