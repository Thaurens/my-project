#include <stdint.h>
#include "inc/tm4c123gh6pm.h"

// LCD port tanımlamaları (örneğin, PA0, PA1, PB0, PB1, PB2, PB3 için)
#define RS 0x01 // PA0
#define E  0x02 // PA1
#define D4 0x10 // PB0
#define D5 0x20 // PB1
#define D6 0x40 // PB2
#define D7 0x80 // PB3

// LCD komutları
#define LCD_CLEAR 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE_SET 0x06
#define LCD_DISPLAY_ON_CURSOR_OFF 0x0C

void LCD_Command(uint8_t cmd) {
    GPIO_PORTA_DATA_R &= ~RS;      // RS = 0 (komut modu)
    GPIO_PORTA_DATA_R &= ~E;       // E = 0 (enable)
    GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & 0x0F) | (cmd & 0xF0); // Yüksek 4 bit
    GPIO_PORTA_DATA_R |= E;        // E = 1 (enable pulse)
    GPIO_PORTA_DATA_R &= ~E;       // E = 0 (enable pulse)

    GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & 0x0F) | (cmd << 4); // Düşük 4 bit
    GPIO_PORTA_DATA_R |= E;        // E = 1 (enable pulse)
    GPIO_PORTA_DATA_R &= ~E;       // E = 0 (enable pulse)
}

void LCD_Data(uint8_t data) {
    GPIO_PORTA_DATA_R |= RS;       // RS = 1 (data modu)
    GPIO_PORTA_DATA_R &= ~E;       // E = 0 (enable)
    GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & 0x0F) | (data & 0xF0); // Yüksek 4 bit
    GPIO_PORTA_DATA_R |= E;        // E = 1 (enable pulse)
    GPIO_PORTA_DATA_R &= ~E;       // E = 0 (enable pulse)

    GPIO_PORTB_DATA_R = (GPIO_PORTB_DATA_R & 0x0F) | (data << 4); // Düşük 4 bit
    GPIO_PORTA_DATA_R |= E;        // E = 1 (enable pulse)
    GPIO_PORTA_DATA_R &= ~E;       // E = 0 (enable pulse)
}

void LCD_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x03; // Port A ve B'ye saat ver
    while ((SYSCTL_PRGPIO_R & 0x03) == 0); // Portların hazır olmasını bekle

    GPIO_PORTA_DIR_R |= (RS | E);  // PA0 ve PA1 çıkış
    GPIO_PORTA_DEN_R |= (RS | E);  // PA0 ve PA1 dijital
    GPIO_PORTB_DIR_R |= (D4 | D5 | D6 | D7);  // PB0, PB1, PB2, PB3 çıkış
    GPIO_PORTB_DEN_R |= (D4 | D5 | D6 | D7);  // PB0, PB1, PB2, PB3 dijital

    LCD_Command(0x02); // 4-bit mod
    LCD_Command(LCD_ENTRY_MODE_SET); // Giriş modu
    LCD_Command(LCD_DISPLAY_ON_CURSOR_OFF); // Ekranda yazı göster, imleç yok
    LCD_Command(LCD_CLEAR); // Ekranı temizle
}

// LCD'ye bir yazı yazma
void LCD_Print(char* str) {
    while (*str) {
        LCD_Data(*str);  // Tek tek karakter gönder
        str++;
    }
}

// ADC Başlatma Fonksiyonu
void ADC0_Init(void) {
    SYSCTL_RCGCADC_R |= 1;           // ADC0 modülüne saat ver
    SYSCTL_RCGCGPIO_R |= 0x10;       // Port E'ye saat ver (PE3 için)
    while ((SYSCTL_PRGPIO_R & 0x10) == 0); // Port E hazır olana kadar bekle

    GPIO_PORTE_DIR_R &= ~0x08;       // PE3 giriş olarak ayarla
    GPIO_PORTE_AFSEL_R |= 0x08;      // PE3 alternatif fonksiyon
    GPIO_PORTE_DEN_R &= ~0x08;       // PE3 dijital fonksiyonu devre dışı bırak
    GPIO_PORTE_AMSEL_R |= 0x08;      // PE3 analog mod

    ADC0_ACTSS_R &= ~8;              // ADC0 örnek dizisi 3'ü devre dışı bırak
    ADC0_EMUX_R &= ~0xF000;          // Yazılım tetiklemeyi seç
    ADC0_SSMUX3_R = 0;               // Kanal 0'u seç (PE3 ADC Kanal 0)
    ADC0_SSCTL3_R = 0x6;             // Sonuç kaydı ve bayrak ayarları
    ADC0_ACTSS_R |= 8;               // ADC0 örnek dizisi 3'ü etkinleştir
}

// ADC değeri okuma fonksiyonu
uint32_t ADC0_Read(void) {
    ADC0_PSSI_R = 8;                  // Örnek alma işlemini başlat
    while ((ADC0_RIS_R & 8) == 0);    // Sonuç hazır olana kadar bekle
    uint32_t result = ADC0_SSFIFO3_R; // Sonucu oku
    ADC0_ISC_R = 8;                   // Bayrağı temizle
    return result;
}

// Sıcaklık hesaplama fonksiyonu
float CalculateTemperature(uint32_t adcValue) {
    float voltage = (adcValue * 3.3) / 4096.0; // ADC değerini voltaja dönüştür
    float temperature = voltage * 100.0;      // Voltajı sıcaklığa çevir (10 mV/°C)
    return temperature;
}

int main(void) {
    volatile uint32_t adcValue;
    volatile float temperature;
    char buffer[16];

    LCD_Init(); // LCD'yi başlat
    ADC0_Init(); // ADC'yi başlat

    while (1) {
        adcValue = ADC0_Read();              // ADC'den değeri oku
        temperature = CalculateTemperature(adcValue); // Sıcaklığı hesapla

        // Sıcaklık değerini string'e dönüştür
        snprintf(buffer, sizeof(buffer), "Temp: %.2f C", temperature);

        LCD_Command(LCD_CLEAR); // Ekranı temizle
        LCD_Print(buffer); // Ekrana yazdır

        for (int i = 0; i < 1000000; i++); // Biraz bekle
    }
}
