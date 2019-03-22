#include "DHT11.h"
#include "delays.h"

void dht11_request(void) {
    DHT11_OUTPUT();
    DHT11_CLR();
    delay_ms(18);
    DHT11_SET();
}

void dht11_response(void) {
    DHT11_INPUT();

    int err = 0;
    while (IOPIN0 & 0x00000010)//wait for HIGH
    {
        err++;
        if (err >= 1000000)return;
    }
    err = 0;

    while ((IOPIN0 & 0x00000010) == 0) //wait for LOW
    {
        err++;
        if (err >= 1000000)return;
    }
    err = 0;

    while (IOPIN0 & 0x00000010) //wait for HIGH
    {
        err++;
        if (err >= 1000000) return;
    }
}

tU8 dht11_data(void) {

    int err = 0;
    int count;
    tU8 data = 0;
    for (count = 0; count < 8; count++)    /* 8 bits of data */
    {
        while ((IOPIN0 & 0x00000010) == 0)    /* Wait till response is LOW */
        {
            err++;
            if (err >= 1000000) return 0;
        }
        err = 0;
        delay_us(30);    /* delay greater than 24 usec */
        if (IOPIN0 & 0x00000010) /* If response is HIGH, 1 is received */
            data = ((data << 1) | 0x01);
        else    /* If response is LOW, 0 is received */
            data = (data << 1);
        while (IOPIN0 & 0x00000010)    /* Wait till response is HIGH (happens if 1 is received) */
        {
            err++;
            if (err >= 1000000) return 0;
        }
        err = 0;
    }

    return data;
}

