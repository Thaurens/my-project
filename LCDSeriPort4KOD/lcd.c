#include "inc/tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/gpio.c"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.c"
#include "lcd.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

void otuzhexgonder()
{
    SysCtlDelay(100000);
    // rs=0
    GPIOPinWrite(LCDPORT, RS, 0);
    //ayar yazıldı
    GPIOPinWrite(LCDPORT, D4|D5|D6|D7, 0X30);
    // en ac kapa
    GPIOPinWrite(LCDPORT, E, 2);
    SysCtlDelay(1000);
    GPIOPinWrite(LCDPORT, E, 0);
    // 3 kez 30h gönder
}

void lcdkomut(unsigned char c) //0x47
{
    GPIOPinWrite(LCDPORT, RS, 0);
    GPIOPinWrite(LCDPORT, D4|D5|D6|D7, (c & 0xf0));
    GPIOPinWrite(LCDPORT, E, 2);
    SysCtlDelay(1000);
    GPIOPinWrite(LCDPORT, E, 0);

    SysCtlDelay(1000);

    GPIOPinWrite(LCDPORT, RS, 0);
    GPIOPinWrite(LCDPORT, D4|D5|D6|D7, (c & 0x0f)<<4);
    GPIOPinWrite(LCDPORT, E, 2);
    SysCtlDelay(1000);
    GPIOPinWrite(LCDPORT, E, 0);

    SysCtlDelay(1000);

}

void LCDilkayarlar()
{
    //portf_base_enable
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOOutput(LCDPORT, 0xFF); // hepsi out

    otuzhexgonder();
    otuzhexgonder();
    otuzhexgonder();

    // ayarlar devam
    //lcd komut kullan
    lcdkomut(0x0f); // ekrani ac-kapa
    SysCtlDelay(6000);
    lcdkomut(0x01); // ekrani sil
    SysCtlDelay(6000);
    lcdkomut(0x06); // giris kipi
    SysCtlDelay(6000);
    lcdkomut(0x02); // kursör basa don
    SysCtlDelay(6000);
    lcdkomut(0x18); // kusör kaydir
    SysCtlDelay(6000);
}

void lcdfonksiyonayarla( unsigned char e)
{
    SysCtlDelay(2000);
    GPIOPinWrite(LCDPORT, RS, 0);
    GPIOPinWrite(LCDPORT, D4|D5|D6|D7, (e & 0xf0));
    GPIOPinWrite(LCDPORT, E, 2);
    SysCtlDelay(2000);
    GPIOPinWrite(LCDPORT, E, 0);

    SysCtlDelay(2000);

    GPIOPinWrite(LCDPORT, RS, 0);
    GPIOPinWrite(LCDPORT, D4|D5|D6|D7, (e & 0x0f)<<4);
    GPIOPinWrite(LCDPORT, E, 2);
    SysCtlDelay(2000);
    GPIOPinWrite(LCDPORT, E, 0);

    SysCtlDelay(2000);

}


void lcdkarakteryaz(unsigned char d)
{
    GPIOPinWrite(LCDPORT, RS, 1); // Karakter yazma modu
    GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7, (d & 0xF0));
    GPIOPinWrite(LCDPORT, E, 2);
    SysCtlDelay(1000);
    GPIOPinWrite(LCDPORT, E, 0);

    SysCtlDelay(1000);

    GPIOPinWrite(LCDPORT, RS, 1); // Karakter yazma modu
    GPIOPinWrite(LCDPORT, D4 | D5 | D6 | D7, (d & 0x0F) << 4);
    GPIOPinWrite(LCDPORT, E, 2);
    SysCtlDelay(1000);
    GPIOPinWrite(LCDPORT, E, 0);

    SysCtlDelay(1000);

}


void LCDgit(unsigned char x, unsigned char y){

        if(x==1)
                lcdkomut(0x80+((y-1)%16));
        else
                lcdkomut(0xC0+((y-1)%16));

}

void LCDTemizle(){
    GPIOPinWrite(LCDPORT, RS, 0);
    SysCtlDelay(2000);
    lcdkomut(0x01);

}

void LCDSaatYaz(int sa, int dk, int sn) {
    char sao;
    char sab;
    char dko;
    char dkb;
    char sno;
    char snb;

    sao = sa / 10 ;
    sab = sa % 10 ;
    dko = dk / 10 ;
    dkb = dk % 10 ;
    sno = sn / 10 ;
    snb = sn % 10 ;

    // Display on LCD, limited to 8 characters
    lcdkarakteryaz(sao + '0');
    lcdkarakteryaz(sab + '0');
    lcdkarakteryaz(':');
    lcdkarakteryaz(dko + '0');
    lcdkarakteryaz(dkb + '0');
    lcdkarakteryaz(':');
    lcdkarakteryaz(sno + '0');
    lcdkarakteryaz(snb + '0');
}


void incrementTotalSeconds() {
    totalSeconds++;
}

int totalSeconds = 0;
