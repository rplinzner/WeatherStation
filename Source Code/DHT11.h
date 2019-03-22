#ifndef _DHT_h
#define _DHT_h
#include <lpc2xxx.h>
#include "general.h"

//------- adres pinu DHT11 ----
#define DHT_PIN 4

#define CLEAR_BIT(byte,bit) ((byte) &= ~(1 << (bit)))
#define DHT11_INPUT() 	(IODIR0 &= ~(1<<DHT_PIN))
#define DHT11_OUTPUT() 	(IODIR0 |= (1<<DHT_PIN))
#define DHT11_SET()			(IOSET0 = (1<<DHT_PIN))
#define DHT11_CLR() 		(IOCLR0 = (1<<DHT_PIN))

void dht11_response(void);
tU8 dht11_data(void);
void dht11_request(void);

#endif
