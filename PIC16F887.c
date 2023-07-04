#include<16f887.h>
#fuses hs, put, noprotect, nowdt
#use delay(clock=20mhz)
//!#fuses intrc_io
//!#use delay(clock=8mhz)
#use spi( SPI1, FORCE_HW, BITS=8, stream=SPI_STREAM)
//!#use rs232(baud=9600, bits=8, stop=1, parity=n, rcv=pin_c7, xmit=pin_c6)
#define btn_up       pin_b0
#define btn_down     pin_b1
#define btn_ok       pin_b2
#define btn_cancel   pin_b3
#define btn_set      pin_b4

#define led1          pin_e0
#define led2          pin_e1
#define led3          pin_e2
#define in1          pin_a0
#define in2          pin_a1
#define in3          pin_a2
#define in4          pin_a3
//!#define ena          pin_c4
//!#define enb          pin_c5

#define DT1 pin_c3
#define SCK pin_c4


#include<lcd.c>

signed int8 thoi_gian_cai_phut_ht_tam=0, thoi_gian_cai_gio_ht_tam=0;
signed int8 thoi_gian_cai_phut_l1_tam=0, thoi_gian_cai_gio_l1_tam=0;
signed int8 thoi_gian_cai_phut_l2_tam=0, thoi_gian_cai_gio_l2_tam=0;
signed int8 thoi_gian_cai_phut_l3_tam=0, thoi_gian_cai_gio_l3_tam=0;
signed int16 khoi_luong_cai_l1_tam=200, khoi_luong_cai_l2_tam=200, khoi_luong_cai_l3_tam=200;

signed int8 phut_ht=-1, gio_ht=-1;
signed int8 phut_l1=-1, gio_l1=-1;
signed int8 phut_l2=-1, gio_l2=-1;
signed int8 phut_l3=-1, gio_l3=-1;
signed int16 khoi_luong_l1=-1, khoi_luong_l2=-1, khoi_luong_l3=-1;


//!unsigned int8 phut=0, gio=0;
//!unsigned int8 gt_phut_hien_tai=0, gt_gio_hien_tai=0;
signed int8 tt_hien_thi=-1;
unsigned int16 bdn=0, bdn_check=0;
unsigned int16 tang_gia_tri=0, giam_gia_tri=0;

unsigned int8 mode=0, tt_lan=0;
signed int8 sobuoc=0;
unsigned int8 bdn_motor=0, motor_delay=1;

unsigned int8 thu_tu_cai=0; //tuong trung cho cai gio va phut


unsigned int16 kl_loadcell;
unsigned int32 read_loadcell = 0, offset = 0;
float gram = 0, SCALE = 213.82;

void nhan_btn_down();
void nhan_btn_set();
void nhan_btn_up();
void nhan_btn_ok();
void nhan_btn_cancel();
void tang_gia_tri();
void giam_gia_tri();
void chon_gia_tri();
void xoa_gia_tri();
void cap_nhat_gia_tri();
void dong_co_quay_thuan();
void dong_co_quay_nghich();
unsigned int32 readCount(void);
int32 readAverage(void);

#int_timer1
void interrupt_timer1()
{ 
   bdn++; 
   bdn_check++;
//!   if((mode==11)||(mode==21)||(mode==31))
//!   {
//!       bdn_motor++;
//!   }
   set_timer1(3036);
}

#int_timer0
void interrupt_timer0()
{ 
   if((tt_lan==1)||(tt_lan==2)||(tt_lan==3))
   {
       bdn_motor++;
   }
   set_timer0(-250); 
}

unsigned int8 stepmotor_fullstep[]={0x0e,0x0b,0x0d,0x07}; 
unsigned int8 stepmotor_i=0; 

void step_motor_quay_thuan_fs() 
{    
   output_a(stepmotor_fullstep[stepmotor_i]);
   stepmotor_i++; 
   stepmotor_i = stepmotor_i & 0x03; ///0x03=>0000 0011// gia tri 0-3: 4 trang thai
} 
void step_motor_quay_nghich_fs() 
{    
   output_a(stepmotor_fullstep[stepmotor_i]);
   stepmotor_i--; 
   stepmotor_i = stepmotor_i & 0x03; ///0x03=>0000 0011// gia tri 0-3: 4 trang thai
}

void main()
{
   set_tris_b(0xff);
   set_tris_c(0b10000000); //RC6: Chan truyen, RC7: Chan nhan
   port_b_pullups(0xff);
   output_a(0x0e);
   lcd_init();
   setup_timer_1(t1_disabled);
   
   setup_timer_0(rtcc_div_8|rtcc_internal);  
   set_timer0(-250);
//!   enable_interrupts(global);
   enable_interrupts(int_timer0);
   
   offset = readAverage();

   
   lcd_gotoxy(5,1);
   lcd_putc("DO AN MON HOC");
   lcd_gotoxy(1,2);
   lcd_putc("GVHD: TRAN DANG KHOA");
   lcd_gotoxy(23,1);
   lcd_putc("SVTH: Phuong Uyen");
   lcd_gotoxy(23,2);
   lcd_putc("      Thao Van");
   
   delay_ms(1000);
   lcd_putc(12);
   lcd_gotoxy(1,1);
   lcd_putc("Vui long nhan SET de");
   lcd_gotoxy(1,2);
   lcd_putc("    cai thoi gian");
   
   while(true)
   {
      nhan_btn_down();
      nhan_btn_up();
      nhan_btn_ok();
      nhan_btn_set();
      nhan_btn_cancel();
      
      
      if (bdn>=600)//bdn=10 tuong ung 1s, bdn=600 tuong ung 1 phut
      { 
         bdn = bdn-600;
         phut_ht++;
         if(phut_ht>=60)
         {
            phut_ht=phut_ht-60;
            gio_ht++;
            if(gio_ht>=24)
            {
               gio_ht=gio_ht-24;
            }
         }
         
         if ((phut_l1==phut_ht)&&(phut_ht!=-1)&&(gio_l1==gio_ht)&&(gio_ht!=-1)&&(khoi_luong_l1!=-1))
         {
            tt_lan=1; mode=1;
            output_high(led1);
         }
         else if((phut_l2==phut_ht)&&(phut_ht!=-1)&&(gio_l2==gio_ht)&&(gio_ht!=-1)&&(khoi_luong_l2!=-1))
         {
            tt_lan=2; mode=1;
            
         }
         else if((phut_l3==phut_ht)&&(phut_ht!=-1)&&(gio_l3==gio_ht)&&(gio_ht!=-1)&&(khoi_luong_l3!=-1))
         {
            tt_lan=3; mode=1;
            output_high(led3);
         }
         
         
         if(tt_hien_thi==4)
         {
            lcd_gotoxy(5,1);
            printf(lcd_putc, "%02d:%02d", gio_ht, phut_ht);
            
         }
      }
      

      if((tt_lan==1)||(tt_lan==2)||(tt_lan==3))
      {
         //////LOADCELL
         read_loadcell = readCount();
         if (offset >= read_loadcell) 
         {
            gram = 0;
         } 
         else
         {
            float val2 = (read_loadcell - offset);
            gram = val2;
         }
         kl_loadcell=gram/SCALE;
         lcd_gotoxy(14,1);
         printf(lcd_putc, "%5lu g", kl_loadcell);
         ///////////////////
      }

      if(tt_lan==1)
      {
         
         if(mode==1) // quay dong co
         {
            if(kl_loadcell<=khoi_luong_l1-80)
            {
               if(sobuoc<=13)  
               {
                  if(bdn_motor>=motor_delay)
                  {  
                     output_high(led2);
                     bdn_motor=0;
                     step_motor_quay_thuan_fs(); 
                     sobuoc++;
//!                     lcd_gotoxy(1,2);
//!                     printf(lcd_putc,"So buoc: %2d", sobuoc);
                  }
               }else;
            } 
            else //if(kl_loadcell>khoi_luong_l1)
            {  output_low(led2);
               if(sobuoc>=0)
               {  
                  if(bdn_motor>=motor_delay)
                  {
                     output_low(led1);
                     bdn_motor=0;
                     step_motor_quay_nghich_fs(); 
                     sobuoc--;
//!                     lcd_gotoxy(1,2);
//!                     printf(lcd_putc,"So buoc: %2d", sobuoc);
                  } 
               } else mode=0;  // dong co ngung quay
            }
         }
      }
    ///===============
      
      if(tt_lan==2)
      {
         if(mode==1) // quay dong co
         {
            if(kl_loadcell<=khoi_luong_l2-80)
            {
               if(sobuoc<=13)  
               {
                  if(bdn_motor>=motor_delay)
                  {  
                     bdn_motor=0;
                     step_motor_quay_thuan_fs(); 
                     sobuoc++;
                  }
               }else;
            } 
            else //if(kl_loadcell>=khoi_luong_l2)
            {
               if(sobuoc>=0)
               {  
                  if(bdn_motor>=motor_delay)
                  {
                     bdn_motor=0;
                     step_motor_quay_nghich_fs(); 
                     sobuoc--;
                  } 
               } else mode=0;  // dong co ngung quay
            }
         }
      }
    ///============
     
      if(tt_lan==3)
      {
         if(mode==1) // quay dong co
         {
            if(kl_loadcell<=khoi_luong_l3-80)
            {
               if(sobuoc<=13)  
               {
                  if(bdn_motor>=motor_delay)
                  {  
                     bdn_motor=0;
                     step_motor_quay_thuan_fs(); 
                     sobuoc++;
                  }
               }else;
            } 
            else //if(kl_loadcell>=khoi_luong_l3)
            {
               if(sobuoc>=0)
               {  
                  if(bdn_motor>=motor_delay)
                  {
                     bdn_motor=0;
                     step_motor_quay_nghich_fs(); 
                     sobuoc--;
                  } 
               } else mode=0;  // dong co ngung quay
            }
         }
      }
      
      /////
      
   }
}





//!CHUONG TRINH CON
void nhan_btn_set()
{ 
   if (input(btn_set)==0) 
   {   
      delay_ms(20); 
      if(input(btn_set)==0) 
      {  
         lcd_putc(12);
         tt_hien_thi++;
         if(tt_hien_thi>=4) tt_hien_thi=0;
         thu_tu_cai=0;
         lcd_gotoxy(21,2);
         lcd_putc("Cai gio         ");
         
         if(tt_hien_thi==0)
         {
            lcd_gotoxy(2,1);
            lcd_putc("THOI GIAN HIEN TAI"); 
         }
         else if((tt_hien_thi>0)&&(tt_hien_thi<4))
         {
            lcd_gotoxy(1,1);
            printf(lcd_putc,"Cho an lan %01u",tt_hien_thi);
            lcd_gotoxy(1,2);
            lcd_putc("T.Gian=  :  ");
            lcd_gotoxy(21,1);
            lcd_putc("K.Luong=      gram  ");
         }
         cap_nhat_gia_tri();
         while(input(btn_set)==0); 
      } 
   } 
}

void nhan_btn_up()
{ 
   if (input(btn_up)==0) 
   {   
      delay_ms(20); 
      if(input(btn_up)==0) 
      {    
         tang_gia_tri();
         cap_nhat_gia_tri();
         while(input(btn_up)==0); 
      } 
   } 
}

void nhan_btn_down()
{ 
   if (input(btn_down)==0) 
   {   
      delay_ms(20); 
      if(input(btn_down)==0) 
      {    
         giam_gia_tri();
         cap_nhat_gia_tri();
         while(input(btn_down)==0); 
      } 
   } 
}

void nhan_btn_ok()
{ 
   if (input(btn_ok)==0) 
   {   
      delay_ms(20); 
      if(input(btn_ok)==0) 
      {    
         bdn_check=0;
         while(input(btn_ok)==0)
         {
            if(bdn_check>=30) // nhan giu phim ok 3s
            {
               thu_tu_cai=3;
               tt_hien_thi=3;
               break;
            }
         }
         chon_gia_tri();
         cap_nhat_gia_tri();
      } 
   } 
}

void nhan_btn_cancel()
{ 
   if (input(btn_cancel)==0) 
   {   
      delay_ms(20); 
      if(input(btn_cancel)==0) 
      {  
         xoa_gia_tri();
         if(tt_hien_thi<4)
         {
            lcd_gotoxy(21,2);
            lcd_putc("Cai gio             ");
         }
         else 
         {
            lcd_putc(12);
            lcd_gotoxy(21,2);
            lcd_putc("Nhan SET de cai lai ");
         }
         thu_tu_cai=0;
         cap_nhat_gia_tri();
         while(input(btn_cancel)==0); 
      } 
   } 
}



void chon_gia_tri()
{
   
   if((tt_hien_thi==0)||(tt_hien_thi==1)||(tt_hien_thi==2)||(tt_hien_thi==3))
   {
      thu_tu_cai++;
   }
   
   if(thu_tu_cai==1)
   {
      lcd_gotoxy(21,2);
      lcd_putc("Cai phut            ");
      if(tt_hien_thi==0)         gio_ht=thoi_gian_cai_gio_ht_tam;
      else if(tt_hien_thi==1)    gio_l1=thoi_gian_cai_gio_l1_tam;
      else if(tt_hien_thi==2)    gio_l2=thoi_gian_cai_gio_l2_tam;
      else if(tt_hien_thi==3)    gio_l3=thoi_gian_cai_gio_l3_tam;
   }
   else if((thu_tu_cai==2)&&(tt_hien_thi==0))
   {
      phut_ht=thoi_gian_cai_phut_ht_tam;
      setup_timer_1(t1_internal|t1_div_by_8); 
      set_timer1(3036); 
      bdn=0;
      enable_interrupts(global);
      enable_interrupts(int_timer1);
      lcd_gotoxy(21,1);
      lcd_putc("Hoan tat cai th.gian");
      lcd_gotoxy(21,2);
      lcd_putc("Nhan SET de tiep tuc");  
   }
   else if(thu_tu_cai==2)
   {
      lcd_gotoxy(21,2);
      lcd_putc("Cai khoi luong      ");
      if(tt_hien_thi==1)         phut_l1=thoi_gian_cai_phut_l1_tam;
      else if(tt_hien_thi==2)    phut_l2=thoi_gian_cai_phut_l2_tam;
      else if(tt_hien_thi==3)    phut_l3=thoi_gian_cai_phut_l3_tam;
   }
   else if((thu_tu_cai==3)&&(tt_hien_thi!=3))
   {
      lcd_gotoxy(21,2);
      lcd_putc("Nhan SET de tiep tuc");
      if(tt_hien_thi==1)         khoi_luong_l1=khoi_luong_cai_l1_tam;
      else if(tt_hien_thi==2)    khoi_luong_l2=khoi_luong_cai_l2_tam;
   }
   else if((thu_tu_cai==3)&&(tt_hien_thi==3))
   {
      lcd_gotoxy(21,2);
      lcd_putc("Nhan OK de hoan tat ");
      khoi_luong_l3=khoi_luong_cai_l3_tam;
   }
   else if((thu_tu_cai==4)&&(tt_hien_thi==3))
   {
      lcd_putc(12);
      tt_hien_thi=4;
      lcd_gotoxy(1,1);
      printf(lcd_putc,"TG: %02d:%02d", gio_ht, phut_ht);
      lcd_gotoxy(1,2);
      if((phut_l1==-1)||(gio_l1==-1)||(khoi_luong_l1==-1))
      {
         lcd_putc("Lan 1 cai loi       ");
         phut_l1=-1; gio_l1=-1; khoi_luong_l1=-1;
      }
      else
      {
         printf(lcd_putc,"Lan 1: %02d:%02d %5lu g", gio_l1, phut_l1, khoi_luong_l1);
         lcd_gotoxy(13,2); lcd_putc(47);
      }
      lcd_gotoxy(21,1);
      if((phut_l2==-1)||(gio_l2==-1)||(khoi_luong_l2==-1))
      {
         lcd_putc("Lan 2 cai loi       ");
         phut_l2=-1; gio_l2=-1; khoi_luong_l2=-1;
      }
      else
      {
         printf(lcd_putc,"Lan 2: %02d:%02d %5lu g", gio_l2, phut_l2, khoi_luong_l2);
         lcd_gotoxy(33,1); lcd_putc(47);
      }
      lcd_gotoxy(21,2);
      if((phut_l3==-1)||(gio_l3==-1)||(khoi_luong_l3==-1))
      {
         lcd_putc("Lan 3 cai loi       ");
         phut_l3=-1; gio_l3=-1; khoi_luong_l3=-1;
      }
      else
      {
         printf(lcd_putc,"Lan 3: %02d:%02d %5lu g", gio_l3, phut_l3, khoi_luong_l3);
         lcd_gotoxy(33,2); lcd_putc(47);
      }
   }
}

void xoa_gia_tri()
{
   if(tt_hien_thi==0)
   {
      thoi_gian_cai_phut_ht_tam=0; thoi_gian_cai_gio_ht_tam=0;
      phut_ht=-1; gio_ht=-1;
   }
   
   else if(tt_hien_thi==1)
   {
      thoi_gian_cai_gio_l1_tam=0; thoi_gian_cai_phut_l1_tam=0;
      khoi_luong_cai_l1_tam=200;
      phut_l1=-1; gio_l1=-1; khoi_luong_l1=-1;
   }
   
   else if(tt_hien_thi==2)
   {
      thoi_gian_cai_gio_l2_tam=0; thoi_gian_cai_phut_l2_tam=0;
      khoi_luong_cai_l2_tam=200;
      phut_l2=-1; gio_l2=-1; khoi_luong_l2=-1;
   }
   
   else if(tt_hien_thi==3)
   {
      thoi_gian_cai_gio_l3_tam=0; thoi_gian_cai_phut_l3_tam=0;
      khoi_luong_cai_l3_tam=200;
      phut_l3=-1; gio_l3=-1; khoi_luong_l3=-1;
   }
   else if(((thu_tu_cai==3)&&(tt_hien_thi==3))||(tt_hien_thi==4))
   {
      thoi_gian_cai_phut_ht_tam=0; thoi_gian_cai_gio_ht_tam=0;
      phut_ht=-1; gio_ht=-1;
      
      thoi_gian_cai_gio_l1_tam=0; thoi_gian_cai_phut_l1_tam=0;
      khoi_luong_cai_l1_tam=200;
      phut_l1=-1; gio_l1=-1; khoi_luong_l1=-1;
   
      thoi_gian_cai_gio_l2_tam=0; thoi_gian_cai_phut_l2_tam=0;
      khoi_luong_cai_l2_tam=200;
      phut_l2=-1; gio_l2=-1; khoi_luong_l2=-1;
      
      thoi_gian_cai_gio_l3_tam=0; thoi_gian_cai_phut_l3_tam=0;
      khoi_luong_cai_l3_tam=200;
      phut_l3=-1; gio_l3=-1; khoi_luong_l3=-1;
   }
}


///////////////////////////////////////////////////////////////////////////////
void cap_nhat_gia_tri()
{
   if(tt_hien_thi==0)
   {
      lcd_gotoxy(9,2);
      if((thoi_gian_cai_gio_ht_tam!=gio_ht)||(thoi_gian_cai_phut_ht_tam!=phut_ht))
      {
         printf(lcd_putc,"%02d:%02d      N",thoi_gian_cai_gio_ht_tam, thoi_gian_cai_phut_ht_tam);
      }

      else if((thoi_gian_cai_gio_ht_tam==gio_ht)&&(thoi_gian_cai_phut_ht_tam==phut_ht))
      {
         printf(lcd_putc,"%02d:%02d      Y",gio_ht, phut_ht);
      }
   }
   else if(tt_hien_thi==1)
   { 
      lcd_gotoxy(8,2);
      if((thoi_gian_cai_gio_l1_tam!=gio_l1)||(thoi_gian_cai_phut_l1_tam!=phut_l1))
      {
         printf(lcd_putc,"%02d:%02d       N",thoi_gian_cai_gio_l1_tam, thoi_gian_cai_phut_l1_tam);
      }
      else if((thoi_gian_cai_gio_l1_tam==gio_l1)&&(thoi_gian_cai_phut_l1_tam==phut_l1))
      {
         printf(lcd_putc,"%02d:%02d       Y",gio_l1, phut_l1);
      }
      
      lcd_gotoxy(29,1);
      if(khoi_luong_cai_l1_tam!=khoi_luong_l1)
      {
         printf(lcd_putc,"%5Lu",khoi_luong_cai_l1_tam);
         lcd_gotoxy(40,1); lcd_putc('N');
      }
      else if(khoi_luong_l1==khoi_luong_cai_l1_tam)
      {
         printf(lcd_putc,"%5Lu",khoi_luong_l1);
         lcd_gotoxy(40,1); lcd_putc('Y');
      }
   }
   else if(tt_hien_thi==2)
   {
      lcd_gotoxy(8,2);
      if((thoi_gian_cai_gio_l2_tam!=gio_l2)||(thoi_gian_cai_phut_l2_tam!=phut_l2))
      {
         printf(lcd_putc,"%02d:%02d       N",thoi_gian_cai_gio_l2_tam, thoi_gian_cai_phut_l2_tam);
      }
      else if((thoi_gian_cai_gio_l2_tam==gio_l2)&&(thoi_gian_cai_phut_l2_tam==phut_l2))
      {
         printf(lcd_putc,"%02d:%02d       Y",gio_l2, phut_l2);
      }
      
      lcd_gotoxy(29,1);
      if(khoi_luong_cai_l2_tam!=khoi_luong_l2)
      {
         printf(lcd_putc,"%5Lu",khoi_luong_cai_l2_tam);
         lcd_gotoxy(40,1); lcd_putc('N');
      }
      else if(khoi_luong_l2==khoi_luong_l2)
      {
         printf(lcd_putc,"%5Lu",khoi_luong_l2);
         lcd_gotoxy(40,1); lcd_putc('Y');
      }
   }
   else if(tt_hien_thi==3)
   {
      lcd_gotoxy(8,2);
      if((thoi_gian_cai_gio_l3_tam!=gio_l3)||(thoi_gian_cai_phut_l3_tam!=phut_l3))
      {
         printf(lcd_putc,"%02d:%02d       N",thoi_gian_cai_gio_l3_tam, thoi_gian_cai_phut_l3_tam);
      }
      else if((thoi_gian_cai_gio_l3_tam==gio_l3)&&(thoi_gian_cai_phut_l3_tam==phut_l3))
      {
         printf(lcd_putc,"%02d:%02d       Y",gio_l3, phut_l3);
      }
      
      lcd_gotoxy(29,1);
      if(khoi_luong_cai_l3_tam!=khoi_luong_l3)
      {
         printf(lcd_putc,"%5Lu",khoi_luong_cai_l3_tam);
         lcd_gotoxy(40,1); lcd_putc('N');
      }
      else if(khoi_luong_cai_l3_tam==khoi_luong_l3)
      {
         printf(lcd_putc,"%5Lu",khoi_luong_l3);
         lcd_gotoxy(40,1); lcd_putc('Y');
      }
   }
   
//!   else if(tt_hien_thi==4)
//!   {
//!      lcd_gotoxy(21,2);
//!      lcd_putc("                    ");
//!   }
}

void tang_gia_tri()
{
   if((tt_hien_thi==0)&&(thu_tu_cai==0))
   {
      thoi_gian_cai_gio_ht_tam++;
      if(thoi_gian_cai_gio_ht_tam==24)
      {
         thoi_gian_cai_gio_ht_tam=0;
      }
   }

   else if((tt_hien_thi==0)&&(thu_tu_cai==1))
   {
      thoi_gian_cai_phut_ht_tam++;
      if(thoi_gian_cai_phut_ht_tam==60)
      {
         thoi_gian_cai_phut_ht_tam=0;
      }
   }
   
   else if((tt_hien_thi==1)&&(thu_tu_cai==0))
   {
      thoi_gian_cai_gio_l1_tam++;
      if(thoi_gian_cai_gio_l1_tam==24)
      {
         thoi_gian_cai_gio_l1_tam=0;
      }
   }

   else if((tt_hien_thi==1)&&(thu_tu_cai==1))
   {
      thoi_gian_cai_phut_l1_tam++;
      if(thoi_gian_cai_phut_l1_tam==60)
      {
         thoi_gian_cai_phut_l1_tam=0;
      }
   }
   
   else if((tt_hien_thi==1)&&(thu_tu_cai==2))
   {
      khoi_luong_cai_l1_tam=khoi_luong_cai_l1_tam+10;
      if(khoi_luong_cai_l1_tam>=10000)
      {
         khoi_luong_cai_l1_tam=100;
      }
   }
   
   else if((tt_hien_thi==2)&&(thu_tu_cai==0))
   {
      thoi_gian_cai_gio_l2_tam++;
      if(thoi_gian_cai_gio_l2_tam==24)
      {
         thoi_gian_cai_gio_l2_tam=0;
      }
   }

   else if((tt_hien_thi==2)&&(thu_tu_cai==1))
   {
      thoi_gian_cai_phut_l2_tam++;
      if(thoi_gian_cai_phut_l2_tam==60)
      {
         thoi_gian_cai_phut_l2_tam=0;
      }
   }
   
   else if((tt_hien_thi==2)&&(thu_tu_cai==2))
   {
      khoi_luong_cai_l2_tam=khoi_luong_cai_l2_tam+10;
      if(khoi_luong_cai_l2_tam>=10000)
      {
         khoi_luong_cai_l2_tam=100;
      }
   }
   
   else if((tt_hien_thi==3)&&(thu_tu_cai==0))
   {
      thoi_gian_cai_gio_l3_tam++;
      if(thoi_gian_cai_gio_l3_tam==24)
      {
         thoi_gian_cai_gio_l3_tam=0;
      }
   }

   else if((tt_hien_thi==3)&&(thu_tu_cai==1))
   {
      thoi_gian_cai_phut_l3_tam++;
      if(thoi_gian_cai_phut_l3_tam==60)
      {
         thoi_gian_cai_phut_l3_tam=0;
      }
   }
   
   else if((tt_hien_thi==3)&&(thu_tu_cai==2))
   {
      khoi_luong_cai_l3_tam=khoi_luong_cai_l3_tam+10;
      if(khoi_luong_cai_l3_tam>=10000)
      {
         khoi_luong_cai_l3_tam=100;
      }
   }
}

void giam_gia_tri()
{
   if((tt_hien_thi==0)&&(thu_tu_cai==0))
   {
      thoi_gian_cai_gio_ht_tam--;
      if(thoi_gian_cai_gio_ht_tam==-1)
      {
         thoi_gian_cai_gio_ht_tam=23;
      }
   }

   else if((tt_hien_thi==0)&&(thu_tu_cai==1))
   {
      thoi_gian_cai_phut_ht_tam--;
      if(thoi_gian_cai_phut_ht_tam==-1)
      {
         thoi_gian_cai_phut_ht_tam=59;
      }
   }
   
   
   else if((tt_hien_thi==1)&&(thu_tu_cai==0))
   {
      thoi_gian_cai_gio_l1_tam--;
      if(thoi_gian_cai_gio_l1_tam==-1)
      {
         thoi_gian_cai_gio_l1_tam=23;
      }
   }

   else if((tt_hien_thi==1)&&(thu_tu_cai==1))
   {
      thoi_gian_cai_phut_l1_tam--;
      if(thoi_gian_cai_phut_l1_tam==-1)
      {
         thoi_gian_cai_phut_l1_tam=59;
      }
   }
   
   else if((tt_hien_thi==1)&&(thu_tu_cai==2))
   {
      khoi_luong_cai_l1_tam=khoi_luong_cai_l1_tam-10;
      if(khoi_luong_cai_l1_tam<100)
      {
         khoi_luong_cai_l1_tam=10000;
      }
   }
   
   else if((tt_hien_thi==2)&&(thu_tu_cai==0))
   {
      thoi_gian_cai_gio_l2_tam--;
      if(thoi_gian_cai_gio_l2_tam==-1)
      {
         thoi_gian_cai_gio_l2_tam=23;
      }
   }

   else if((tt_hien_thi==2)&&(thu_tu_cai==1))
   {
      thoi_gian_cai_phut_l2_tam--;
      if(thoi_gian_cai_phut_l2_tam==-1)
      {
         thoi_gian_cai_phut_l2_tam=59;
      }
   }
   
   else if((tt_hien_thi==2)&&(thu_tu_cai==2))
   {
      khoi_luong_cai_l2_tam=khoi_luong_cai_l2_tam-10;
      if(khoi_luong_cai_l2_tam<100)
      {
         khoi_luong_cai_l2_tam=10000;
      }
   }
   
   else if((tt_hien_thi==3)&&(thu_tu_cai==0))
   {
      thoi_gian_cai_gio_l3_tam--;
      if(thoi_gian_cai_gio_l3_tam==-1)
      {
         thoi_gian_cai_gio_l3_tam=23;
      }
   }

   else if((tt_hien_thi==3)&&(thu_tu_cai==1))
   {
      thoi_gian_cai_phut_l3_tam--;
      if(thoi_gian_cai_phut_l3_tam==-1)
      {
         thoi_gian_cai_phut_l3_tam=59;
      }
   }
   
   else if((tt_hien_thi==3)&&(thu_tu_cai==2))
   {
      khoi_luong_cai_l3_tam=khoi_luong_cai_l3_tam-10;
      if(khoi_luong_cai_l3_tam<100)
      {
         khoi_luong_cai_l3_tam=10000;
      }
   }
}


unsigned int32 readCount(void) 
{
   unsigned int32 data;
   unsigned int8 j;
   output_high(DT1);
   output_low(SCK);
   data = 0;
   
   while (input(DT1)); //DT1==1
   for (j = 0; j < 24; j++) 
   {
      output_high(SCK);
      data = data << 1;
      output_low(SCK);
      if (input(DT1)) 
      {
         data++;
      }
   }
   output_high(SCK);
   data = data ^ 0x800000;
   output_low(SCK);
   return data;
}

int32 readAverage(void) 
{
   unsigned int32 sum = 0;
   for (int k = 0; k < 20; k++) 
   {
      sum = sum + readCount();
   }
   sum = sum / 20;
   return sum;
}

