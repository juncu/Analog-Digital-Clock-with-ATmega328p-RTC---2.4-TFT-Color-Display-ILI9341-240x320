/*
 * gju_ili9341_clock.c
 *
 * Created: 27.02.2020 10:37:23
 *  Author: gju
 */ 


#define F_CPU 16000000	// Has to be here (not in .h or program.c) otherwise fucked up things happen
#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>	// strlen()

#include "gju_ili9341_clock.h"

#include "./includes/wiring.h"
#include "./includes/uart.h"

#include "./includes/ili9341.h"
#include "./includes/ili9341cmd.h"
#include "./includes/DS3231.h"

//#define _DEBUG

#define _CENTER_X 120
#define _CENTER_Y 160

/***  micro- Clock variables   ***/
float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
float sdeg=0, mdeg=0, hdeg=0;
//static uint8_t center_x=120,center_y=120;                    
uint16_t osx=_CENTER_X, osy=_CENTER_Y, omx=_CENTER_X, omy=_CENTER_Y, ohx=_CENTER_X, ohy=_CENTER_Y;  // Saved H, M, S x & y coords
//uint16_t x0=0, x1=0, y0=0, y1=0;
static uint16_t x0, x1, y0, y1;
/*************************************/

/***  micro- miliseconds variables  ***/
// Interval is how long we wait
static unsigned long currentMillis;

// Tracks the time since last event fired
static unsigned long previousMillis=0;
/*************************************/

uint8_t previous_sec=0,ss,mm,hh,day,date,month,age,year;                    //Clock variables 
//uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time
bool initial = 1; 
	

static char rx_Buffer[MAX_STRING]="";
int count=0;

char sSec[3]="";
char sMin[3]="";
char sHour[3]="";


char sDate[3]="";
char sMonth[3]="";
char sYear[3]="";


static char Data_Buffer[11]="";
static char m_sDate[11]="";
static char m_sTime[11]="";


static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
/* itoa:  convert n to characters in s */
void conv_itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void create_time_string(void)
{
	conv_itoa(hh,sHour);
	if(strlen(sHour)==1){
		m_sTime[0]='0';
		m_sTime[1]=sHour[0];
	}
	else{	
		m_sTime[0]=sHour[0];
		m_sTime[1]=sHour[1];
	}	
	m_sTime[2]=':';//0x2E;
	conv_itoa(mm,sMin);
	if(strlen(sMin)==1){
		m_sTime[3]='0';
		m_sTime[4]=sMin[0];
	}
	else{	
		m_sTime[3]=sMin[0];
		m_sTime[4]=sMin[1];
	}	
	m_sTime[5]=':';
	conv_itoa(ss,sSec);
	if(strlen(sSec)==1){
		m_sTime[6]='0';
		m_sTime[7]=sSec[0];
	}
	else{	
		m_sTime[6]=sSec[0];
		m_sTime[7]=sSec[1];
	}
	m_sTime[8]='\0';
}
void create_date_string(void)
{
	conv_itoa(date,sDate);
	if(strlen(sDate)==1){
		m_sDate[0]='0';
		m_sDate[1]=sDate[0];
	}
	else{	
		m_sDate[0]=sDate[0];
		m_sDate[1]=sDate[1];
	}	
	m_sDate[2]='-';//0x2E;
	conv_itoa(month,sMonth);
	if(strlen(sMonth)==1){
		m_sDate[3]='0';
		m_sDate[4]=sMonth[0];
	}
	else{	
		m_sDate[3]=sMonth[0];
		m_sDate[4]=sMonth[1];
	}	
	m_sDate[5]='-';
	conv_itoa(year,sYear);
	m_sDate[6]='2';
	m_sDate[7]='0';
	m_sDate[8]=sYear[0];
	m_sDate[9]=sYear[1];
	m_sDate[10]='\0';
}
ISR(USART_RX_vect)
{
	uint8_t data;
	data=UDR0;
	
	rx_Buffer[count++]=data;
	if(data=='\r' || count>MAX_STRING){
		
		rx_Buffer[count]='\0';
		if(strstr(rx_Buffer,"set")!=0){
			if(memcmp(rx_Buffer, "set", 3)==0)
			{
				cli();
				memmove(rx_Buffer,rx_Buffer+4,strlen(rx_Buffer)+1);
				
					hh = conv2d(rx_Buffer);
					mm = conv2d(rx_Buffer + 3);
					ss = conv2d(rx_Buffer + 6); // Get H, M, S from compile time

					setTime(hh, mm, ss, -1, 0);
					//setDate(day, date, month,age, year);

					refresh_time();	
					
				printf("SET %d",rx_Buffer);
				sei();
			}
		}
		
		count=0;
	}
}

void Draw_clock_face(){

	// Draw clock face
	drawCircle(_CENTER_X, _CENTER_Y, 118, TFT_GREEN);
	drawCircle(_CENTER_X, _CENTER_Y, 110, TFT_BLACK);    
	
	// Draw 12 lines
	for(int i = 0; i<360; i+= 30) {
		sx = cos((i-90)*0.0174532925);
		sy = sin((i-90)*0.0174532925);
		x0 = sx*114+_CENTER_X;
		y0 = sy*114+_CENTER_Y;
		x1 = sx*100+_CENTER_X;
		y1 = sy*100+_CENTER_Y;
		drawLine(x0, y0, x1, y1, TFT_GREEN);
	}	
	

	// Draw 60 dots
	for(int i = 0; i<360; i+= 6) {
		sx = cos((i-90)*0.0174532925);
		sy = sin((i-90)*0.0174532925);
		x0 = sx*102+_CENTER_X;
		y0 = sy*102+_CENTER_Y;
		// Draw minute markers
		drawPixel(x0, y0, TFT_WHITE);
		// Draw main quadrant dots
    
		if(i==0 || i==180) 
			fillCircle(x0, y0, 2, TFT_WHITE);
		if(i==90 || i==270)
			fillCircle(x0, y0, 2, TFT_WHITE);
	}

	fillCircle(_CENTER_X, _CENTER_Y+1, 3, TFT_RED);
}


void refresh_time(){
	//WriteCmdData(ILI9341_CMD_IDLE_MODE_ON,0); 

	getTime(&hh, &mm, &ss, 0, 0); 

	//WriteCmdData(ILI9341_CMD_IDLE_MODE_OFF,1); 

	
	begin(_lcd_ID);
	setRotation(2);  //LANDSCAPE
	fillScreen(TFT_GREY);

	Draw_clock_face();

	//setTextColorWithBgn(TFT_WHITE, TFT_GREY); // Adding a background colour erases previous text automatically
	
	show_time();
	
}

void refresh_date(){
	WriteCmdData(ILI9341_CMD_IDLE_MODE_ON,0); 

	getDate(&day, &date, &month,&age, &year);

	WriteCmdData(ILI9341_CMD_IDLE_MODE_OFF,1); 

	begin(_lcd_ID);
	setRotation(2);  //LANDSCAPE
	fillScreen(TFT_GREY);

	Draw_clock_face();

	//setTextColorWithBgn(TFT_WHITE, TFT_GREY); // Adding a background colour erases previous text automatically
	show_date();	
}

void show_time(){
	
	useStroke=true;

	strokeColor=TFT_WHITE;
	//setTextWrap(true);

	setTextSize(2); //!must
	setTextColor(TFT_GREY,TFT_GREY);
	setCursor(70,100);
	setTextColor(TFT_WHITE,TFT_GREY);
	setCursor(70,100);
	create_time_string();
	#ifdef _DEBUG
	printf(m_sTime);	
	#endif // _DEBUG
	print(m_sTime,strlen(m_sTime));	
}

void show_date(){

	//setRotation(1);  //LANDSCAPE

	useStroke=true;

	strokeColor=TFT_WHITE;
	//setTextWrap(true);

	setTextSize(3); //!must
	setTextColor(TFT_WHITE,TFT_GREY);
	setCursor(30,10);
	create_date_string();
	#ifdef _DEBUG
	printf(m_sDate);	
	#endif // _DEBUG
	print(m_sDate,strlen(m_sDate));
	//setRotation(2);  //LANDSCAPE

}

FILE uart_str = FDEV_SETUP_STREAM(uart_putch, uart_getch, _FDEV_SETUP_RW);

int main(void)
{
    
	// Set the PORTB-PINB5 as Output:
	DDRB=0b00100000;
	PORTB=0x0b00100000;
	
	stdout = stdin = &uart_str;// Define Output/Input Stream

	init(); //initialize TCNT0

	DS3231_init(); 

	USART_Init( MYUBRR );
	printf("USART OK !");

	getTime(&hh, &mm, &ss, 0, 0);
	getDate(&day, &date, &month,&age, &year);

	/********    ID = 0x9325 ***********/
	
	uint16_t _lcd_ID = tft_readID();       
	begin(_lcd_ID);

	setRotation(2);  //LANDSCAPE
	fillScreen(TFT_GREY);
	
	//setTextColorWithBgn(TFT_WHITE, TFT_GREY); // Adding a background colour erases previous text automatically
	
	Draw_clock_face();
	
  // Draw text at position 120,260 using fonts 4
  // Only font numbers 2,4,6,7 are valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : . - a p m
  // Font 7 is a 7 segment font and only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .
  
  //gju todo tft.drawCentreString("Time flies",120,260,4);
 
	show_date();
	show_time();

	// add const if this should never change
	unsigned long interval=1000L;
	currentMillis=0;

	while(1)
    {
		
        //TODO:: Please write your application code
		/*********** each second ******************/
		//delay(tick_sec);
		
		//ss++;


		
		if (ss==60) {

			ss=0;
			//initial=1;
			mm++;            // Advance minute
			
			
			if(mm>59) {
				
				mm=0;
                initial=1;
				hh++;          // Advance hour
				if (hh>23) {

					// for time accurancy 
					refresh_time();

					refresh_date();
					hh=0;
					initial=1;
				}
			}
		}		
		
		// Get snapshot of time
		currentMillis=millis();

		
		if ((unsigned long)(currentMillis - previousMillis) >= interval)
		{ 
			
			
			if (previous_sec != ss || initial)
			{
				//cli();
				if (hh==0 && initial ){
					refresh_date();
					refresh_time();
					initial=0;  				
				}
				else if (mm==0 && initial ){
					refresh_time();
					show_date();
					initial=0;  				
				}
				else
					show_time();	 

				drawLine(osx, osy, _CENTER_X, _CENTER_Y+1, TFT_GREY);   //erase second hand every seconds
				//WriteCmdData(ILI9341_CMD_NOP,0); 
			
				// Erase hour and minute hand positions every minute
				drawLine(ohx, ohy, _CENTER_X, _CENTER_Y+1, TFT_GREY);
				ohx = hx * 62 + (_CENTER_X+1);    
				ohy = hy * 62 + (_CENTER_Y+1);
				drawLine(omx, omy, _CENTER_X, _CENTER_Y+1, TFT_GREY);
				omx = mx * 84 + _CENTER_X;    
				//omx = mx * 84 + _CENTER_X+1;    
				omy = my * 84 + _CENTER_Y+1;

     			// Redraw new hand positions, hour and minute hands not erased here to avoid flicker
				osx = (sx * 90 + (_CENTER_X+1));    
				osy = (sy * 90 + _CENTER_Y+1);
				drawLine(osx, osy, _CENTER_X, _CENTER_Y+1, TFT_RED);   //second hand
				drawLine(ohx, ohy, _CENTER_X, _CENTER_Y+1, TFT_WHITE); //hour hand
				drawLine(omx, omy, _CENTER_X, _CENTER_Y+1, TFT_WHITE); // minute hand

				fillCircle(_CENTER_X, _CENTER_Y+1, 3, TFT_RED);
				#ifdef _DEBUG
				printf("%2.2d\n",ss);
				#endif // _DEBUG
            
				initial=0;
				//ss++;
				previous_sec=ss;
			}
			
			ss++;
			PORTB ^= (1<<PINB5);
			previousMillis = currentMillis;

			//sei();
		
		}		
		//WriteCmdData(ILI9341_CMD_SLEEP_OUT,0); 

			// Pre-compute hand degrees, x & y coords for a fast screen update
			sdeg = ss * 6;                  // 0-59 -> 0-354
			mdeg = mm * 6 + sdeg*0.01666667;  // 0-59 -> 0-360 - includes seconds
			hdeg = hh * 30 + mdeg*0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds
			//float offset=90+40;
			float offset=90;
		
			hx = cos((hdeg-offset)*0.0174532925);    
			hy = sin((hdeg-offset)*0.0174532925);
			mx = cos((mdeg-offset)*0.0174532925);    
			my = sin((mdeg-offset)*0.0174532925);
			sx = cos((sdeg-offset)*0.0174532925);    
			sy = sin((sdeg-offset)*0.0174532925);


	
  }
}

