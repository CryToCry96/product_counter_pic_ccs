/*
 Project: Dem san pham (V1.0)
 MCU: PIC16F628A
 Clock: 4.0MHz External resonator
 Chinh sua lan cuoi 17/09/2011
 PIC16F628 Khong ho tro I2C nen khong the dung bo nho EEPROM ngoai
*/
#include <16F628A.h>
#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used FOR I/O
#use delay(crystal=4M)          //Dung thach anh 4MHz (4MHz thoi nhe, ko cao hon)

//LCD Pins
#define LCD_ENABLE_PIN  PIN_B6
#define LCD_RS_PIN      PIN_B7
#define LCD_RW_PIN      PIN_B1
#define LCD_DATA4       PIN_B5
#define LCD_DATA5       PIN_B4
#define LCD_DATA6       PIN_B3
#define LCD_DATA7       PIN_B2
#include <lcd.c> //LCD 16x2

//Custom def
#define lcd_clear() lcd_putc("\f") //Minh thich thi minh define cho de nho thoi :))

//////////////////Main code o ben duoi nay nhe/////////////////
//Global
int32 product = 0;

//Kiem tra thong so chung
void run_first()
{
   lcd_gotoxy(1,1); //LCD: cot 1 dong 1
   lcd_putc("Hello");
   delay_ms(1000);
   lcd_gotoxy(1,2); //LCD: cot 1 dong 2
   if(input(PIN_A0)) lcd_putc("Cam bien: OK!"); //Kiem tra tin hieu vao
   else
   {
      while(!input(PIN_A0)) //Lap vo han den khi het loi cam bien
      {
         lcd_gotoxy(3,2); //LCD: cot 3 dong 2
         lcd_putc("Loi cam bien!");
         delay_ms(500);
         lcd_clear();
         delay_ms(500);
      }
      lcd_putc("Cam bien: OK!");
   }
}

//Dem tin hieu tu cam bien
int32 count()
{
   int covered = 0;
   while(input(PIN_A0))
   {
      //chi dem 1 lan
      if(covered == 0) product ++;
      write_eeprom(1, product);//Ghi so san pham vao EEPROM
      covered = 1;
   }
   return product;
}

//ham chinh, bao gom ca setup
void main()
{
   lcd_init(); //Init lcd16x2

   //Pins setup
   set_tris_a(0x1); //Input
   int mode    = 0;
   
   run_first(); //Kiem tra cam bien da :)
   delay_ms(500);
   lcd_clear();
   
   //Product counter
   while(true)
   {  
      if(input(PIN_A1)) mode = 1; //Start
      else
      if(input(PIN_A2) && mode == 1) mode = 2; //Pause
      else
      if(input(PIN_A3)) mode = 3;//Dung ket qua lan truoc
      
      lcd_gotoxy(1,1); //LCD: cot 1 dong 1
      switch(mode)
      {
         case 1:
         {
            lcd_gotoxy(1,1); //LCD: cot 1 dong 1
            lcd_putc("So san pham la:");
            lcd_gotoxy(1,2);
            printf(lcd_putc, "%lu SP", count());//chuyen tu int sang string (long unsigned 32bit)
            break;
         }
         case 2:
         {
            lcd_clear();
            lcd_gotoxy(1,1); //LCD: cot 1 dong 1
            lcd_putc("Tam dung!");
            printf(lcd_putc, "%lu SP", count());//chuyen tu int sang string (long unsigned 32bit)
            break;
         }
         case 3:
         {
            product = read_eeprom(1);//Doc so san pham da luu truoc do tu EEPROM
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

