/*
 Project: Dem san pham (V2.0)
 MCU: PIC16F883
 Clock: 20.0MHz External resonator
 Chinh sua lan cuoi 21/10/2017
*/
#include <16F883.h>
#FUSES NOWDT                     // khong su dung bo Watch Dog Timer
#FUSES HS                        // Dao dong tan so cao (> 4mhz voi PCM/PCH) (>10mhz voi PCD)
#FUSES NOBROWNOUT                // Khong reset chip khi BrownOut
#FUSES NOCPD                     // Khong bao ve du lieu EEPROM
#FUSES NOWRT                     // Program memory not write protected
#FUSES NODEBUG                   // Khong su dung che do Debug voi ICD
#FUSES NOPROTECT                 // Cho phep doc lai Code
#device *=16 ADC=10                   // Chon ADC 10bit, 5V=1023
#use delay(crystal=20M)          // Dung thach anh 20MHz

//#use FAST_IO(C)                // Thiet lap che do fast I/O cho PORTC, yeu cau phai chi ro huong Vao/Ra 
//#CASE                          // Phan biet chu hoa va chu thuong trong khi lap trinh code

#define USE_EXT_EEPROM           // Su dung EEPROM ngoai (external), comment de su dung EEPROM noi (internal)

#ifdef USE_EXT_EEPROM //24LC01 EEPROM
//#define EEPROM_SDA  PIN_C4
//#define EEPROM_SCL  PIN_C3
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
#include <lcd.c> //LCD 16x2, dung thu vien mac dinh cua CCS C

//Custom def
//Minh thich thi minh define cho de nho thoi :))
#define lcd_clear() lcd_putc("\f") 
//////////////////Main code o ben duoi nay nhe/////////////////
//Global values
int32 product = 0;   //Khoi tao bien luu so luong san pham
int mode = 0; //Bien cuc bo de lua chon chuc nang
int first_run = 1; //Bien tam thoi de xac dinh khoi tao PWM

//Kiem tra thong so chung
void check_info()
{
   lcd_gotoxy(1,1); //LCD: cot 1 dong 1
   lcd_putc("Tran Van Lam");
   delay_ms(500);
   lcd_clear();
   lcd_putc("Le Viet Hoang");
   delay_ms(500);
   lcd_clear();
   lcd_putc("Hoang Van Huan");
   delay_ms(500);
   /*
   for(int i = 0; i<16; i++)
   {
      lcd_dich_phai();
      delay_ms(200);
   }*/
   lcd_clear();
   lcd_gotoxy(1,2); //LCD: cot 1 dong 2
   if(!input(PIN_B0)) lcd_putc("Cam bien: OK!"); //Kiem tra tin hieu vao
   else
   {
      while(input(PIN_B0)) //Lap vo han den khi het loi cam bien
      {
         lcd_gotoxy(3,2); //LCD: cot 3 dong 2
         lcd_putc("Loi cam bien!");
         delay_ms(500);
         lcd_clear();
         delay_ms(500);
      }
      product = 0; //Reset product sau khi ngat RB0
      lcd_putc("Cam bien: OK!");
/*
#ifdef USE_EXT_EEPROM //24LC01 EEPROM
      while(!ext_eeprom_ready()) //Lap vo han den khi het loi EEPROM
      {
         lcd_gotoxy(1,2); //LCD: cot 3 dong 2
         lcd_putc("Loi ket noi EEPROM!");
         delay_ms(500);
         lcd_clear();
         delay_ms(500);
      }
#endif //24LC01 EEPROM END
*/
   }
}
//Dieu khien toc do bang chuyen
void pwm_control()
{
   if(first_run == 1)//Chi khoi tao PWM 1 lan
   {
      setup_ccp1(CCP_PWM);//Khoi tao PWM tai CCP1 (PIN_C2)
      setup_timer_2(T2_DIV_BY_16, 255, 1); //Tan so 1220Hz
      first_run = 0;
   }
   //du lieu adc max la 1024 tuong ung value = 1024, D = value/(4*(255+1))
   set_pwm1_duty(read_adc()+1); // Dat do rong xung theo ADC, neu < 1 thi dat mac dinh chu ki la 50%
}

//Dem tin hieu tu cam bien dua vao tin hieu ngat pin B0
#int_EXT
void ISR_count()
{
   clear_interrupt(INT_EXT);
   product++;//Tang gia tri len 1 tuong ung voi so san pham
#ifdef USE_EXT_EEPROM //24LC01 EEPROM
   if(product != read_ext_eeprom(1)) //Han che so lan ghi vao EEPROM, chi ghi neu co su thay doi
      write_ext_eeprom(1, product);//Ghi so san pham vao EEPROM
#else
   if(product != read_eeprom(1)) //Han che so lan ghi vao EEPROM, chi ghi neu co su thay doi
      write_eeprom(1,product);
#endif //24LC01 EEPROM END
   delay_ms(3);
}

//ham chinh, bao gom ca setup
void main()
{
   lcd_init();//Khoi tao LCD
   
   //Pins setup
   set_tris_a(0b00001111); //Input PIN_A0
   set_tris_b(0b00000001); //Input PIN B0
   
#ifdef USE_EXT_EEPROM //24LC01 EEPROM
   init_ext_eeprom(); //Khoi tao eeprom 24
#endif //24LC01 EEPROM END   
   setup_adc(ADC_CLOCK_DIV_32); //Thoi gian lay mau = xung clock / 32  ( mat 0.4 us trên thach anh 20MHz )
   setup_adc_ports(sAN0); // Su dung PIN_A0 lam pin lay tin hieu
   set_adc_channel(0);//Chon kenh ADC la 0
   
   ext_int_edge(L_TO_H);//Suon tin hieu tu Cao xuong Thap
   enable_interrupts(INT_EXT); //Ngat ngoai tai pin B0
   enable_interrupts(GLOBAL);// Ngat toan cuc
   delay_ms(10);
   check_info(); //Kiem tra cam bien da :)
   delay_ms(1000);
   lcd_clear();
   
   lcd_putc("Vui long chon");
   lcd_gotoxy(1,2);
   lcd_putc("che do");
   while(TRUE) //Loop forever
   {  
      //Doc nut bam chuc nang
      if(!input(PIN_A1)) 
      { 
         delay_ms(500);
         if(!input(PIN_A1))
         {
            mode = 1;//Start
            lcd_clear();
         }
      }
      if(!input(PIN_A2)) 
      { 
         delay_ms(500);
         if(!input(PIN_A2) && mode == 1) 
         {
            mode = 2;//Pause
            lcd_clear();
         }
      }
      if(!input(PIN_A3)) 
      { 
         delay_ms(500);
         if(!input(PIN_A3))
         {
            mode = 3;//Return
            lcd_clear();
         }
      }
   
      lcd_gotoxy(1,1); //LCD: cot 1 dong 1
      switch(mode)
      {
         case 1:
         {
            lcd_gotoxy(1,1); //LCD: cot 1 dong 1
            lcd_putc("So san pham la:");
            lcd_gotoxy(1,2); //LCD: cot 1 dong 2
            printf(lcd_putc, "%lu SP", product);//chuyen tu int sang string (long unsigned 32bit)
            pwm_control();//Khoi dong bang chuyen
            break;
         }
         case 2:
         {
            
            lcd_gotoxy(1,1); //LCD: cot 1 dong 1
            lcd_putc("Tam dung!");
            lcd_gotoxy(1,2); //LCD: cot 1 dong 2
            printf(lcd_putc, "%lu SP", product);//chuyen tu int sang string (long unsigned 32bit)
            setup_ccp1(CCP_OFF);//Tam dung bang chuyen
            first_run = 1;//Thong bao cho pwm_control() biet la can khoi tao lai PWM
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
      }
   }
}
