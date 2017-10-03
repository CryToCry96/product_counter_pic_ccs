/*
 Project: Dem san pham (V1.0)
 MCU: PIC16F883
 Clock: 20.0MHz External resonator
 Chinh sua lan cuoi 23/09/2017
*/
#include <16F883.h>
#FUSES NOWDT                     // khong su dung bo Watch Dog Timer
#FUSES HS                        // Dao dong tan so cao (> 4mhz voi PCM/PCH) (>10mhz voi PCD)
#FUSES NOBROWNOUT                // Khong reset chip khi BrownOut
#FUSES NOCPD                     // Khong bao ve du lieu EEPROM
#FUSES NOWRT                     // Program memory not write protected
#FUSES NODEBUG                   // Khong su dung che do Debug voi ICD
#FUSES NOPROTECT                 // Cho phep doc lai Code
#device ADC=10                   // Chon ADC 10bit, 5V=1023
#use delay(crystal=20M)          // Dung thach anh 20MHz

//#use FAST_IO(C)                // Thiet lap che do fast I/O cho PORTC, yeu cau phai chi ro huong Vao/Ra 
//#CASE                          // Phan biet chu hoa va chu thuong trong khi lap trinh code

#define USE_EXT_EEPROM           // Su dung EEPROM ngoai (external), comment dong duoi de su dung EEPROM noi (internal)

#ifdef USE_EXT_EEPROM //24LC01 EEPROM
#include <2401.C> //24LC01 EEPROM, delay 3ms
#endif //24LC01 EEPROM END

//LCD Pins
#define LCD_ENABLE_PIN  PIN_B5
#define LCD_RS_PIN      PIN_B7
#define LCD_RW_PIN      PIN_B6
#define LCD_DATA4       PIN_B4
#define LCD_DATA5       PIN_B3
#define LCD_DATA6       PIN_B2
#define LCD_DATA7       PIN_B1
#include <lcd.c> //LCD 16x2

//Custom def
#define lcd_clear() lcd_putc("\f") //Minh thich thi minh define cho de nho thoi :))

//////////////////Main code o ben duoi nay nhe/////////////////
//Global values
int32 product = 0;                           //Khoi tao bien luu so luong san pham
int16 laser_value = 5;                       //(Volt) Gia tri dien ap do duoc khi co anh sang laser, max 5V, can kiem tra thuc te
int16 adc_value = laser_value * 1023 / 5;    //chuyen tu gia tri dien ap sang gia tri ADC, 1V = 204.6

//Kiem tra thong so chung
void check_info()
{
   lcd_gotoxy(1,1); //LCD: cot 1 dong 1
   lcd_putc("Hello");
   delay_ms(1000);
   lcd_gotoxy(1,2); //LCD: cot 1 dong 2
   if(read_adc(7) >= adc_value) lcd_putc("Cam bien: OK!"); //Kiem tra tin hieu vao
   else
   {
      while(read_adc(6) < adc_value) //Lap vo han den khi het loi cam bien
      {
         lcd_gotoxy(3,2); //LCD: cot 3 dong 2
         lcd_putc("Loi cam bien!");
         delay_ms(500);
         lcd_clear();
         read_adc(1);
         delay_ms(500);
      }
      lcd_putc("Cam bien: OK!");
/*#ifdef USE_EXT_EEPROM //24LC01 EEPROM
      while(!ext_eeprom_ready()) //Lap vo han den khi het loi EEPROM
      {
         lcd_gotoxy(1,2); //LCD: cot 3 dong 2
         lcd_putc("Loi ket noi EEPROM!");
         delay_ms(500);
         lcd_clear();
         delay_ms(500);
      }
#endif //24LC01 EEPROM END */
   }
}

//Dem tin hieu tu cam bien
int32 count()
{
   int covered = 0;
   while(read_adc(7) >= adc_value)//Kiem tra gia tri ADC, chong rung
   {
      //chi dem 1 lan
      if(!input(PIN_C0) || !input(PIN_C1) || !input(PIN_C2)) break;
      if(covered == 0) product ++;
#ifdef USE_EXT_EEPROM //24LC01 EEPROM
      write_ext_eeprom(1, product);//Ghi so san pham vao EEPROM
#else
      write_eeprom(1,product);
#endif //24LC01 EEPROM END
      covered = 1;
   }
   return product;
}

//ham chinh, bao gom ca setup
void main()
{
   lcd_init(); //Init lcd16x2

   //Pins setup
   set_tris_a(0b00000001); //Input
   set_tris_c(0b00000111); //Input
   output_float(PIN_C0);
   output_float(PIN_C1);
   output_float(PIN_C2);
#ifdef USE_EXT_EEPROM //24LC01 EEPROM
   init_ext_eeprom(); //Khoi tao eeprom 24
#endif //24LC01 EEPROM END   
   setup_adc(ADC_CLOCK_DIV_2); //Thoi gian lay mau = xung clock / 2  ( mat 0.4 us trên thach anh 20MHz )
   setup_adc_ports(1); // Su dung PIN_A0 lam pin lay tin hieu
   delay_us(10);
   
   int mode    = 0;
   int first_run = 1;
   
   check_info(); //Kiem tra cam bien da :)
   delay_ms(500);
   lcd_clear();
   
   //Product counter
   while(TRUE) //Loop forever
   {  
      if(!input(PIN_C0)) mode = 1; //Start
      else
      if(!input(PIN_C1) && mode == 1) mode = 2; //Pause
      else
      if(!input(PIN_C2)) mode = 3;//Dung ket qua lan truoc
      
      lcd_gotoxy(1,1); //LCD: cot 1 dong 1
      switch(mode)
      {
         case 1:
         {
            lcd_gotoxy(1,1); //LCD: cot 1 dong 1
            lcd_putc("So san pham la:");
            lcd_gotoxy(1,2); //LCD: cot 1 dong 2
            if(first_run)
            {
               printf(lcd_putc, "%lu SP", product);//chuyen tu int sang string (long unsigned 32bit)
               first_run = 0;
            }
            else
            printf(lcd_putc, "%lu SP", count());//chuyen tu int sang string (long unsigned 32bit)
            break;
         }
         case 2:
         {
            lcd_clear();
            lcd_gotoxy(1,1); //LCD: cot 1 dong 1
            lcd_putc("Tam dung!");
            //lcd_gotoxy(1,2); //LCD: cot 1 dong 2
            //printf(lcd_putc, "%lu SP", count());//chuyen tu int sang string (long unsigned 32bit)
            break;
         }
         case 3:
         {
#ifdef USE_EXT_EEPROM //24LC01 EEPROM
            product = read_ext_eeprom(1);//Doc so san pham da luu truoc do tu EEPROM
#else
            product = read_eeprom(1);
#endif //24LC01 EEPROM END
            lcd_gotoxy(1,1); //LCD: cot 1 dong 1
            lcd_putc("Dung ket qua lan");
            lcd_gotoxy(1,2); //LCD: cot 1 dong 1
            lcd_putc("truoc");
            delay_ms(2000);
            mode = 1; //tro ve mode 1
            lcd_clear();
            break;
         }
         case 0: //Chon chuc nang (default)
         {
            lcd_putc("Vui long chon");
            lcd_gotoxy(1,2);
            lcd_putc("che do");
            delay_ms(250);
            lcd_clear();
            delay_ms(250);
            break;
         }
      }
   }
}

