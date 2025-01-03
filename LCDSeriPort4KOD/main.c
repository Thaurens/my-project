#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/uart.c"
#include "driverlib/pin_map.h"

#define RS GPIO_PIN_0
#define E  GPIO_PIN_1
#define D4 GPIO_PIN_4
#define D5 GPIO_PIN_5
#define D6 GPIO_PIN_6
#define D7 GPIO_PIN_7

void SetInitSettings();
void timerkesmefonksiyonu();
void delayMs(int n);
void lcd_command(unsigned char command);
void lcd_data(unsigned char data);
void lcd_init(void);
void lcd_print(char *str);
void format_time(char *buffer, int saat, int dakika, int saniye);

void uart_ayari();
void diger_ayar();
void serikesme();

// Zaman değişkenleri (kesme fonksiyonu için volatile)
int saat;
int dakika;
int saniye;

int main(void)
{
    lcd_init();          // LCD'yi başlat
    SetInitSettings();   // Timer ve kesmeleri ayarla
    uart_ayari();
    diger_ayar();

    while (1)
    {
    }
}

void SetInitSettings()
{
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN); // 40 MHz

    // GPIO ayarları
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3); // PF1, PF2, PF3 çıkış

    // Timer ayarları
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC); // Periyodik mod
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()); // 40 MHz -> 1 saniye

    // Kesme ayarları
    IntMasterEnable();                             // Genel kesmeleri aç
    IntEnable(INT_TIMER0A);                        // Timer0A kesmesini etkinleştir
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Kesme kaynağını seç
    TimerIntRegister(TIMER0_BASE, TIMER_A, timerkesmefonksiyonu); // Kesme işleyicisini kaydet
    //TimerEnable(TIMER0_BASE, TIMER_A);             // Timer'ı başlat


}

// Kesme işleyicisi
void timerkesmefonksiyonu()
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Kesme bayrağını temizle

    // Zamanı güncelle
    saniye++;
    if (saniye == 60) {
        saniye = 0;
        dakika++;
    }
    if (dakika == 60) {
        dakika = 0;
        saat++;
    }
    if (saat == 24) {
        saat = 0;
    }

    // Zamanı string formatında UART üzerinden gönder
    char buffer2[9]; // "HH:MM:SS" için yer ayır
    format_time(buffer2, saat, dakika, saniye); // Formatlanmış zamanı oluştur
    int i;
    for (i = 0; i < 8; i++) {
        UARTCharPut(UART0_BASE, buffer2[i]);
    }

    // Zamanı LCD'ye yazdır
    lcd_command(0xC8); // 2. satıra git
    lcd_print(buffer2);
}

// LCD başlatma
void lcd_init() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, (RS | E | D4 | D5 | D6 | D7));

    delayMs(20);
    lcd_command(0x03);
    delayMs(5);
    lcd_command(0x03);
    delayMs(5);
    lcd_command(0x03);
    delayMs(1);
    lcd_command(0x02);
    delayMs(1);

    lcd_command(0x28); // 4-bit mod, 2 satır
    delayMs(1);
    lcd_command(0x0C); // Ekranı aç, imleç kapalı
    delayMs(1);
    lcd_command(0x06); // Yazma yönü sağa
    delayMs(1);
    lcd_command(0x01); // Ekranı temizle
    delayMs(2);
}

// LCD komut gönderme
void lcd_command(unsigned char command) {

    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (command & 0xF0));
    GPIOPinWrite(GPIO_PORTB_BASE, RS, 0);
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);
    delayMs(2);
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0);

    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (command << 4));
    GPIOPinWrite(GPIO_PORTB_BASE, RS, 0);
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);
    delayMs(2);
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0);
}

// LCD veri gönderme
void lcd_data(unsigned char data) {
    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (data & 0xF0));
    GPIOPinWrite(GPIO_PORTB_BASE, RS, RS);
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);
    delayMs(2);
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0);

    GPIOPinWrite(GPIO_PORTB_BASE, D4 | D5 | D6 | D7, (data << 4));
    GPIOPinWrite(GPIO_PORTB_BASE, RS, RS);
    GPIOPinWrite(GPIO_PORTB_BASE, E, E);
    delayMs(2);
    GPIOPinWrite(GPIO_PORTB_BASE, E, 0);
}

// LCD'ye string yazdırma
void lcd_print(char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

// Zamanı formatlama
void format_time(char *buffer, int saat, int dakika, int saniye) {
    buffer[0] = '0' + (saat / 10);
    buffer[1] = '0' + (saat % 10);
    buffer[2] = ':';
    buffer[3] = '0' + (dakika / 10);
    buffer[4] = '0' + (dakika % 10);
    buffer[5] = ':';
    buffer[6] = '0' + (saniye / 10);
    buffer[7] = '0' + (saniye % 10);
    buffer[8] = '\0';
}

// Gecikme fonksiyonu
void delayMs(int n)
{
    SysCtlDelay((n * SysCtlClockGet()) / 3000);
}

void uart_ayari()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // pin ayarları
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, 255);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // UART0 yapılandır
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600, // Baud rate: 9600
                        (UART_CONFIG_WLEN_8 |               // 8-bit veri uzunluğu
                         UART_CONFIG_STOP_ONE |             // 1 stop biti
                         UART_CONFIG_PAR_NONE));

}

void diger_ayar()
{
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX|UART_INT_RT);
    UARTIntClear(UART0_BASE, UART_INT_RX|UART_INT_RT);
    UARTIntRegister(UART0_BASE, serikesme);
}

void serikesme()
{
    char buffer[9];  // "HH:MM:SS" için 8 karakter + null terminator
    int i = 0;

    // UART üzerinden 8 karakter oku
    while (i < 8) {
        buffer[i] = UARTCharGet(UART0_BASE);
        i++;
    }
    buffer[8] = '\0'; // String'i sonlandır

    // "HH:MM:SS" formatındaki string'i parçala
    saat   = (buffer[0] - '0') * 10 + (buffer[1] - '0');
    dakika = (buffer[3] - '0') * 10 + (buffer[4] - '0');
    saniye = (buffer[6] - '0') * 10 + (buffer[7] - '0');

    TimerEnable(TIMER0_BASE, TIMER_A);
}


