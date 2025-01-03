#ifndef LCD_H_
#define LCD_H_

#define LCDPORT                          GPIO_PORTB_BASE
#define RS                               GPIO_PIN_0
#define E                                GPIO_PIN_1

#define D4                               GPIO_PIN_4
#define D5                               GPIO_PIN_5
#define D6                               GPIO_PIN_6
#define D7                               GPIO_PIN_7

void lcdkomut(unsigned char c); // 0 la 255 arasında veri
// rs=0
// d4-d7 arasında veri yaz
// ve en ac kapa

void lcdkarakteryaz(unsigned char d);
// rs=1
// d4-d7 arasında veri yaz
// ve en ac kapa

void LCDilkayarlar();

//Giriş kipini seç 0 0 0 0 0 0 0 1 I/D S 40µs
//Ekran aç/kapa 0 0 0 0 0 0 1 D C B 40µs
//Kursör kaydır 0 0 0 0 0 1 S/C R/L * * 40µs

void lcdfonksiyonayarla( unsigned char e); // dl,N,F sadece fonskiyon ayarı için
// dl= 0 1
// n=  0 1
// f=  0 1

void LCDTemizle();
// rs=0
// d4-d7 arasında veri yaz 01h
// ve en ac kapa

void LCDgit(unsigned char x, unsigned char y);
// satır ve sütün
// 1,5
// 85

//2,7
// c7

void LCDSaatYaz(int sa, int dk, int sn);


#define DURATION_IN_SECONDS 86400  // Change this value as needed
extern int totalSeconds;



#endif /* LCD_H_ */
