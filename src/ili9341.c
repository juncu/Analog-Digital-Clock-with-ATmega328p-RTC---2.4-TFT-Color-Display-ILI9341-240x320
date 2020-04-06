#include "../includes/ili9341.h"
#include "../includes/ili9341_gju.h"
#include <util/delay.h>
#include <stdbool.h>
#include <stddef.h>
#include "../includes/ili9341cmd.h"
#include <stdio.h> //for FILE

extern const uint16_t PROGMEM port_to_mode_PGM[]; 
extern const uint16_t PROGMEM port_to_input_PGM[];
extern const uint16_t PROGMEM port_to_output_PGM[];
extern const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[]; 

extern int16_t ili9341_putchar_printf(char var, FILE *stream);//this function will be called whenever printf is used


#define SUPPORT_0139              //not working +238 bytes
#define SUPPORT_0154              //S6D0154 +320 bytes
#define SUPPORT_1289              //costs about 408 bytes
#define SUPPORT_1580              //R61580 Untested
#define SUPPORT_1963              //only works with 16BIT bus anyway
#define SUPPORT_4532              //LGDP4532 +120 bytes.  thanks Leodino
#define SUPPORT_4535              //LGDP4535 +180 bytes
#define SUPPORT_68140             //RM68140 +52 bytes defaults to PIXFMT=0x55
#define SUPPORT_7781              //ST7781 +172 bytes
#define SUPPORT_8230              //UC8230 +118 bytes
#define SUPPORT_8347D             //HX8347-D, HX8347-G, HX8347-I +520 bytes, 0.27s
#define SUPPORT_8347A             //HX8347-A +500 bytes, 0.27s
#define SUPPORT_8352A             //HX8352A +486 bytes, 0.27s
#define SUPPORT_8352B             //HX8352B UNTESTED
#define SUPPORT_9225              //ILI9225-B, ILI9225-G ID=0x9225, ID=0x9226
#define SUPPORT_9326_5420         //ILI9326, SPFD5420 +246 bytes
#define SUPPORT_9342              //costs +114 bytes
#define SUPPORT_9806
#define SUPPORT_9488_555          //costs +230 bytes, 0.03s / 0.19s
#define SUPPORT_B509_7793         //R61509, ST7793 +244 bytes
#define OFFSET_9327 32            //costs about 103 bytes, 0.08s



#define MIPI_DCS_REV1   (1<<0)
#define AUTO_READINC    (1<<1)
#define READ_BGR        (1<<2)
#define READ_LOWHIGH    (1<<3)
#define READ_24BITS     (1<<4)
#define XSA_XEA_16BIT   (1<<5)
#define READ_NODUMMY    (1<<6)
#define INVERT_GS       (1<<8)
#define INVERT_SS       (1<<9)
#define MV_AXIS         (1<<10)
#define INVERT_RGB      (1<<11)
#define REV_SCREEN      (1<<12)
#define FLIP_VERT       (1<<13)
#define FLIP_HORIZ      (1<<14)


static uint8_t done_reset, is8347, is555;

uint8_t ss,mm,hh,day,date,month,age,year;                    //Clock variables 
/*
	uint16_t (int16_t x, int16_t y) { 
		uint16_t color; 
		readGRAM(x, y, &color, 1, 1); 
		return color; 
	}
*/

/*
 static const uint16_t ILI9325_regValues[] PROGMEM = {
            0x00E5, 0x78F0,     // set SRAM internal timing
            0x0001, 0x0100,     // set Driver Output Control
            0x0002, 0x0200,     // set 1 line inversion
            0x0003, 0x1030,     // set GRAM write direction and BGR=1.
            0x0004, 0x0000,     // Resize register
            0x0005, 0x0000,     // .kbv 16bits Data Format Selection
            0x0008, 0x0207,     // set the back porch and front porch
            0x0009, 0x0000,     // set non-display area refresh cycle ISC[3:0]
            0x000A, 0x0000,     // FMARK function
            0x000C, 0x0000,     // RGB interface setting
            0x000D, 0x0000,     // Frame marker Position
            0x000F, 0x0000,     // RGB interface polarity
            // ----------- Power On sequence ----------- //
            0x0010, 0x0000,     // SAP, BT[3:0], AP, DSTB, SLP, STB
            0x0011, 0x0007,     // DC1[2:0], DC0[2:0], VC[2:0]
            0x0012, 0x0000,     // VREG1OUT voltage
            0x0013, 0x0000,     // VDV[4:0] for VCOM amplitude
            0x0007, 0x0001,
            TFTLCD_DELAY, 200,  // Dis-charge capacitor power voltage
            0x0010, 0x1690,     // SAP=1, BT=6, APE=1, AP=1, DSTB=0, SLP=0, STB=0
            0x0011, 0x0227,     // DC1=2, DC0=2, VC=7
            TFTLCD_DELAY, 50,   // wait_ms 50ms
            0x0012, 0x000D,     // VCIRE=1, PON=0, VRH=5
            TFTLCD_DELAY, 50,   // wait_ms 50ms
            0x0013, 0x1200,     // VDV=28 for VCOM amplitude
            0x0029, 0x000A,     // VCM=10 for VCOMH
            0x002B, 0x000D,     // Set Frame Rate
            TFTLCD_DELAY, 50,   // wait_ms 50ms
            0x0020, 0x0000,     // GRAM horizontal Address
            0x0021, 0x0000,     // GRAM Vertical Address
            // ----------- Adjust the Gamma Curve ----------//

            0x0030, 0x0000,
            0x0031, 0x0404,
            0x0032, 0x0003,
            0x0035, 0x0405,
            0x0036, 0x0808,
            0x0037, 0x0407,
            0x0038, 0x0303,
            0x0039, 0x0707,
            0x003C, 0x0504,
            0x003D, 0x0808,

            //------------------ Set GRAM area ---------------//
            0x0060, 0x2700,     // Gate Scan Line GS=0 [0xA700] 
            0x0061, 0x0001,     // NDL,VLE, REV .kbv
            0x006A, 0x0000,     // set scrolling line
            //-------------- Partial Display Control ---------//
            0x0080, 0x0000,
            0x0081, 0x0000,
            0x0082, 0x0000,
            0x0083, 0x0000,
            0x0084, 0x0000,
            0x0085, 0x0000,
            //-------------- Panel Control -------------------//
            0x0090, 0x0010,
            0x0092, 0x0000,
            0x0007, 0x0133,     // 262K color and display ON
        };*/
        
/*		
static void writecmddata(uint16_t cmd, uint16_t dat)
{
    CS_ACTIVE;
    WriteCmd(cmd);
    WriteData(dat);
    CS_IDLE;
}
*/

void WriteCmdData(uint16_t cmd, uint16_t dat)
{
    CS_ACTIVE;
    WriteCmd(cmd);
    WriteData(dat);
    CS_IDLE;
}


static void WriteCmdParamN(uint16_t cmd, int8_t N, uint8_t * block)
{
    CS_ACTIVE;
    WriteCmd(cmd);
    while (N-- > 0) {
        uint8_t u8 = *block++;
        CD_DATA;
        write8(u8);
        if (N && is8347) {
            cmd++;
            WriteCmd(cmd);
        }
    }
    CS_IDLE;
}

static inline void WriteCmdParam4(uint8_t cmd, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4)
{
    uint8_t d[4];
    d[0] = d1, d[1] = d2, d[2] = d3, d[3] = d4;
    WriteCmdParamN(cmd, 4, d);
}

static uint16_t color565_to_555(uint16_t color) {
    return (color & 0xFFC0) | ((color & 0x1F) << 1) | ((color & 0x01));  //lose Green LSB, extend Blue LSB
}
static uint16_t color555_to_565(uint16_t color) {
    return (color & 0xFFC0) | ((color & 0x0400) >> 5) | ((color & 0x3F) >> 1); //extend Green LSB
}

#define TFTLCD_DELAY 0xFFFF
#define TFTLCD_DELAY8 0x7F

uint16_t color565(uint8_t r, uint8_t g, uint8_t b) 
{ 
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}




void tft_reset()
{
	done_reset = 1;	
	setWriteDir();
	CTL_INIT();
    CS_IDLE;
    RD_IDLE;
    WR_IDLE;
    RESET_IDLE;
    _delay_ms(50);
    RESET_ACTIVE;
    _delay_ms(100);
    RESET_IDLE;
    _delay_ms(300); //gju wichtig

}

void setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1)
{
#if defined(OFFSET_9327)
	if (_lcd_ID == 0x9327) {
	    if (rotation == 2) y += OFFSET_9327, y1 += OFFSET_9327;
	    if (rotation == 3) x += OFFSET_9327, x1 += OFFSET_9327;
    }
#endif
#if 1
    if (_lcd_ID == 0x1526 && (rotation & 1)) {
		int16_t dx = x1 - x, dy = y1 - y;
		if (dy == 0) { y1++; }
		else if (dx == 0) { x1 += dy; y1 -= dy; }
    }
#endif
    if (_lcd_capable & MIPI_DCS_REV1) {
#if 1
        CS_ACTIVE;    //-0.26s, +272B
        //gju WriteCmd(_MC); write8(x>>8); write8(x); write8(x1>>8); write8(x1); 
        //gju WriteCmd(_MP); write8(y>>8); write8(y); write8(y1>>8); write8(y1);
        CS_IDLE;		
#else
        WriteCmdParam4(_MC, x >> 8, x, x1 >> 8, x1);
        WriteCmdParam4(_MP, y >> 8, y, y1 >> 8, y1);
#endif
    } else {
        WriteCmdData(_MC, x);
        WriteCmdData(_MP, y);
        if (!(x == x1 && y == y1)) {  //only need MC,MP for drawPixel
            if (_lcd_capable & XSA_XEA_16BIT) {
                if (rotation & 1)
                    y1 = y = (y1 << 8) | y;
                else
                    x1 = x = (x1 << 8) | x;
            }
            WriteCmdData(_SC, x);
            WriteCmdData(_SP, y);
            WriteCmdData(_EC, x1);
            WriteCmdData(_EP, y1);
        }
    }
}

void vertScroll(int16_t top, int16_t scrollines, int16_t offset)
{
#if defined(OFFSET_9327)
	if (_lcd_ID == 0x9327) {
	    if (rotation == 2 || rotation == 3) top += OFFSET_9327;
    }
#endif
    int16_t bfa = HEIGHT - top - scrollines;  // bottom fixed area
    int16_t vsp;
    int16_t sea = top;
	if (_lcd_ID == 0x9327) bfa += 32;
    if (offset <= -scrollines || offset >= scrollines) offset = 0; //valid scroll
	vsp = top + offset; // vertical start position
    if (offset < 0)
        vsp += scrollines;          //keep in unsigned range
    sea = top + scrollines - 1;
    if (_lcd_capable & MIPI_DCS_REV1) {
        uint8_t d[6];           // for multi-byte parameters
/*
        if (_lcd_ID == 0x9327) {        //panel is wired for 240x432 
            if (rotation == 2 || rotation == 3) { //180 or 270 degrees
                if (scrollines == HEIGHT) {
                    scrollines = 432;   // we get a glitch but hey-ho
                    vsp -= 432 - HEIGHT;
                }
                if (vsp < 0)
                    vsp += 432;
            }
            bfa = 432 - top - scrollines;
        }
*/
        d[0] = top >> 8;        //TFA
        d[1] = top;
        d[2] = scrollines >> 8; //VSA
        d[3] = scrollines;
        d[4] = bfa >> 8;        //BFA
        d[5] = bfa;
        WriteCmdParamN(is8347 ? 0x0E : 0x33, 6, d);
//        if (offset == 0 && rotation > 1) vsp = top + scrollines;   //make non-valid
		d[0] = vsp >> 8;        //VSP
        d[1] = vsp;
        WriteCmdParamN(is8347 ? 0x14 : 0x37, 2, d);
		if (is8347) { 
		    d[0] = (offset != 0) ? (_lcd_ID == 0x8347 ? 0x02 : 0x08) : 0;
			WriteCmdParamN(_lcd_ID == 0x8347 ? 0x18 : 0x01, 1, d);  //HX8347-D
		} else if (offset == 0 && (_lcd_capable & MIPI_DCS_REV1)) {
			WriteCmdParamN(0x13, 0, NULL);    //NORMAL i.e. disable scroll
		}
		return;
    }
    // cope with 9320 style variants:
    switch (_lcd_ID) {
    case 0x7783:
        WriteCmdData(0x61, _lcd_rev);   //!NDL, !VLE, REV
        WriteCmdData(0x6A, vsp);        //VL#
        break;
#ifdef SUPPORT_0139
    case 0x0139:
        WriteCmdData(0x41, sea);        //SEA
        WriteCmdData(0x42, top);        //SSA
        WriteCmdData(0x43, vsp - top);  //SST
        break;
#endif
#if defined(SUPPORT_0154) || defined(SUPPORT_9225)  //thanks tongbajiel
    case 0x9225:
    case 0x9226:
	case 0x0154:
        WriteCmdData(0x31, sea);        //SEA
        WriteCmdData(0x32, top);        //SSA
        WriteCmdData(0x33, vsp - top);  //SST
        break;
#endif
#ifdef SUPPORT_1289
    case 0x1289:
        WriteCmdData(0x41, vsp);        //VL#
        break;
#endif
	case 0x5420:
    case 0x7793:
	case 0x9326:
	case 0xB509:
        WriteCmdData(0x401, (1 << 1) | _lcd_rev);       //VLE, REV 
        WriteCmdData(0x404, vsp);       //VL# 
        break;
    default:
        // 0x6809, 0x9320, 0x9325, 0x9335, 0xB505 can only scroll whole screen
        WriteCmdData(0x61, (1 << 1) | _lcd_rev);        //!NDL, VLE, REV
        WriteCmdData(0x6A, vsp);        //VL#
        break;
    }
}


void setRotation(uint8_t r)
{
    uint16_t GS, SS, ORG, REV = _lcd_rev;
    uint8_t val;// /*, d[3]*/;
    rotation = r & 3;           // just perform the operation ourselves on the protected variables
    
	_width = (rotation & 1) ? HEIGHT : WIDTH;
    _height = (rotation & 1) ? WIDTH : HEIGHT;
    
	switch (rotation) {
    case 0:                    //PORTRAIT:
		//val = (1 << MEM_X) | (1 << MEM_BGR);
        val = 0b00011000;   /****************OK *******/          //MY=0, MX=0, MV=0, ML=1, BGR=1, MH , XX 
                //MY=0, MX=0, MV=0, ML=1, BGR=1, MH=0 , XX

		//val = (1 << MEM_X) | (0<<MEM_Y) | (0<<MEM_V) | (0<<MEM_L) | (1<<MEM_BGR);
        //val = 0x48;             //MY=0, MX=1, MV=0, ML=0, BGR=1
        break;
    case 1:                    //LANDSCAPE: 90 degrees
      	//val = (1<<MEM_Y)  | (0<<MEM_X)  | (1<<MEM_V) | (0<<MEM_L)  | (1<<MEM_BGR); // ...   
     	//val = ~(1<<MEM_Y)  | ~(1<<MEM_X)  | (1<<MEM_V) | ~(1<<MEM_L)  | (1<<MEM_BGR); // ...   
        //val = 0x28;             //MY=0, MX=0, MV=1, ML=0, BGR=1
        val = 0b00111000;   /****************OK *******/          //MY=1, MX=0, MV=0, ML=1, BGR=1, MH , XX 
                //MY=0, MX=0, MV=1, ML=1, BGR=1, MH=0 , XX
        break;
    case 2:                    //PORTRAIT_REV: 180 degrees
	    //val = (1 << MEM_Y) | (1 << MEM_BGR);                                       // ...   
     	//val != (0<<MEM_X)  | (1<<MEM_Y) | (0<<MEM_V) | (1<<MEM_L)  | (1<<MEM_BGR); // ...   
        val = 0b11011000;   /****************OK *******/          //MY=1, MX=0, MV=0, ML=1, BGR=1, MH , XX 
                //MY=1, MX=1, MV=0, ML=1, BGR=1, MH=0 , XX
		break;
    case 3:                    //LANDSCAPE_REV: 270 degrees
     	val = (1<<MEM_Y)  | (1<<MEM_X)  | (1<<MEM_V) | (1<<MEM_L)  | (1<<MEM_BGR); // ...   
        //val = (1 << MEM_Y) | (1<<MEM_X) | (1<<MEM_V) | (1 << MEM_BGR);             // ...
        //val = 0xF8;             //MY=1, MX=1, MV=1, ML=1, BGR=1
        val = 0b11111000;   /****************OK *******/          //MY=1, MX=0, MV=0, ML=1, BGR=1, MH , XX 
                //MY=0, MX=0, MV=1, ML=1, BGR=1, MH=0 , XX
        break;
    default:
         _MC = 0x20, _MP = 0x21, _MW = 0x22, _SC = 0x50, _EC = 0x51, _SP = 0x52, _EP = 0x53;
          GS = (val & 0x80) ? (1 << 15) : 0;
          WriteCmdData(0x60, GS | 0x2700);    // Gate Scan Line (0xA700)
    }

    if (_lcd_capable & INVERT_GS)
        val ^= 0x80;
    if (_lcd_capable & INVERT_SS)
        val ^= 0x40;
    if (_lcd_capable & INVERT_RGB)
        val ^= 0x08;
    if (_lcd_capable & MIPI_DCS_REV1) {
     // common_MC:
        _MC = 0x2A, _MP = 0x2B, _MW = 0x2C, _SC = 0x2A, _EC = 0x2A, _SP = 0x2B, _EP = 0x2B;
     // common_BGR:
        WriteCmdParamN(is8347 ? 0x16 : 0x36, 1, &val);
        _lcd_madctl = val;
    }
    else {
            _MC = 0x20, _MP = 0x21, _MW = 0x22, _SC = 0x50, _EC = 0x51, _SP = 0x52, _EP = 0x53;
            GS = (val & 0x80) ? (1 << 15) : 0;
            WriteCmdData(0x60, GS | 0x2700);    // Gate Scan Line (0xA700)
            SS = (val & 0x40) ? (1 << 8) : 0;
            WriteCmdData(0x01, SS);     // set Driver Output Control
            ORG = (val & 0x20) ? (1 << 3) : 0;

			if (val & 0x08)
                ORG |= 0x1000;  //BGR
            _lcd_madctl = ORG | 0x0030;
            WriteCmdData(0x03, _lcd_madctl);    // set GRAM write direction and BGR=1.
    }
    if ((rotation & 1) && ((_lcd_capable & MV_AXIS) == 0)) {
//    if ((rotation & 1)) {
        uint16_t x;
        x = _MC, _MC = _MP, _MP = x;
        x = _SC, _SC = _SP, _SP = x;    //.kbv check 0139
        x = _EC, _EC = _EP, _EP = x;    //.kbv check 0139
    }
    
	//setAddrWindow(0, 0, width() - 1, height() - 1);
	setAddrWindow(0, 0, _width - 1, _height - 1);
    
	vertScroll(0, HEIGHT, 0);   //reset scrolling after a rotation
	
}

/******  Fast *****/
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    int16_t end;
#if defined(SUPPORT_9488_555)
    if (is555) color = color565_to_555(color);
#endif
    if (w < 0) {
        w = -w;
        x -= w;
    }                           //+ve w
    end = x + w;
    if (x < 0)
        x = 0;
    if (end > width())
        end = width();
    w = end - x;
    if (h < 0) {
        h = -h;
        y -= h;
    }                           //+ve h
    end = y + h;
    if (y < 0)
        y = 0;
    if (end > height())
        end = height();
    h = end - y;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
//    setAddrWindow(x, y, x + w - 2, y + h - 2);
    CS_ACTIVE;
    WriteCmd(_MW);
    if (h > w) {
        end = h;
        h = w;
        w = end;
    }
    uint8_t hi = color >> 8, lo = color & 0xFF;
    while (h-- > 0) {
        end = w;
#if USING_16BIT_BUS
#if defined(__SAM3X8E__)
#define STROBE_16BIT {WR_ACTIVE;WR_ACTIVE;WR_ACTIVE;WR_IDLE;WR_IDLE;}
#else
#define STROBE_16BIT {WR_ACTIVE; WR_IDLE;}
#endif
        write_16(color);        //we could just do the strobe
        lo = end & 7;
        hi = end >> 3;
        if (hi)
            do {
                STROBE_16BIT;
                STROBE_16BIT;
                STROBE_16BIT;
                STROBE_16BIT;
                STROBE_16BIT;
                STROBE_16BIT;
                STROBE_16BIT;
                STROBE_16BIT;
            } while (--hi > 0);
        while (lo-- > 0) {
            STROBE_16BIT;
        }
#else
        do {
            write8(hi);
            write8(lo);
        } while (--end != 0);
#endif
    }
    CS_IDLE;
    if (!(_lcd_capable & MIPI_DCS_REV1) || ((_lcd_ID == 0x1526) && (rotation & 1)))
        setAddrWindow(0, 0, width() - 1, height() - 1);
}

// Used to do circles and roundrects
void fillCircleHelper(int16_t x0, int16_t y0, int16_t r,uint8_t cornername, int16_t delta, uint16_t color)
{

  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}
 


void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  drawFastVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
} 


void fillScreen(uint16_t color) 
{
  fillRect(0, 0, _width, _height, color);
} 

void drawPixel(int16_t x, int16_t y, uint16_t color)
{
    // MCUFRIEND just plots at edge if you try to write outside of the box:
   

    if (x < 0 || y < 0 || x >= width() || y >= height())
        return;
    //printf("_MC:%d\n",_MC);
    if (_lcd_capable & MIPI_DCS_REV1) {
        WriteCmdParam4(_MC, x >> 8, x, x >> 8, x);
        WriteCmdParam4(_MP, y >> 8, y, y >> 8, y);
    } else {
        WriteCmdData(_MC, x);
        WriteCmdData(_MP, y);
    }
    WriteCmdData(_MW, color);
}



//gjuvoid invertDisplay(boolean i)
void invertDisplay(bool i)
{
    uint8_t val;
    _lcd_rev = ((_lcd_capable & REV_SCREEN) != 0) ^ i;
    if (_lcd_capable & MIPI_DCS_REV1) {
        if (is8347) {
            // HX8347D: 0x36 Panel Characteristic. REV_Panel
            // HX8347A: 0x36 is Display Control 10
            if (_lcd_ID == 0x8347 || _lcd_ID == 0x5252) // HX8347-A, HX5352-A
			    val = _lcd_rev ? 6 : 2;       //INVON id bit#2,  NORON=bit#1
            else val = _lcd_rev ? 8 : 10;     //HX8347-D, G, I: SCROLLON=bit3, INVON=bit1
            // HX8347: 0x01 Display Mode has diff bit mapping for A, D 
            WriteCmdParamN(0x01, 1, &val);
        } else
            WriteCmdParamN(_lcd_rev ? 0x21 : 0x20, 0, NULL);
        return;
    }
    // cope with 9320 style variants:
    switch (_lcd_ID) {
#ifdef SUPPORT_0139
    case 0x0139:
#endif
    case 0x9225:                                        //REV is in reg(0x07) like Samsung
    case 0x9226:
    case 0x0154:
        WriteCmdData(0x07, 0x13 | (_lcd_rev << 2));     //.kbv kludge
        break;
#ifdef SUPPORT_1289
    case 0x1289:
        _lcd_drivOut &= ~(1 << 13);
        if (_lcd_rev)
            _lcd_drivOut |= (1 << 13);
        WriteCmdData(0x01, _lcd_drivOut);
        break;
#endif
	case 0x5420:
    case 0x7793:
    case 0x9326:
	case 0xB509:
        WriteCmdData(0x401, (1 << 1) | _lcd_rev);       //.kbv kludge VLE 
        break;
    default:
        WriteCmdData(0x61, _lcd_rev);
        break;
    }
}



//#define WriteCmdParam4(cmd, d1, d2, d3, d4) {uint8_t d[4];d[0] = d1, d[1] = d2, d[2] = d3, d[3] = d4;WriteCmdParamN(cmd, 4, d);}
void pushCommand(uint16_t cmd, uint8_t * block, int8_t N)
{ 
	WriteCmdParamN(cmd, N, block); 
}


static void init_table16(const void *table, int16_t size)
{

    uint16_t *p = (uint16_t *) table;
    while (size > 0) {
        uint16_t cmd = pgm_read_word(p++);
        uint16_t d = pgm_read_word(p++);
        if (cmd == TFTLCD_DELAY){
          // //gju delay(d);
			if(d==200){
				_delay_ms(200);
			}
			else if(d==50){
				_delay_ms(50);
			}
			else
				_delay_ms(2);
		}
        else {
            CS_ACTIVE;
            WriteCmd(cmd);
            WriteData(d);
            CS_IDLE;
        }
        size -= 2 * sizeof(int16_t);
    }
}
static uint16_t read16bits(void)
{
    uint16_t ret;
    uint8_t lo;
#if USING_16BIT_BUS
    READ_16(ret);               //single strobe to read whole bus
    if (ret > 255)              //ID might say 0x00D3
        return ret;
#else
    READ_8(ret);
#endif
    //all MIPI_DCS_REV1 style params are 8-bit
    READ_8(lo);
    return (ret << 8) | lo;
}

int16_t width(void) 
{ 
  return _width; 
}
 
int16_t height(void) 
{ 
  return _height; 
} 





uint16_t readReg(uint16_t reg, int8_t index)
{
    uint16_t ret;
    uint8_t lo;
    if (!done_reset)
        tft_reset();
    
	CS_ACTIVE;
    WriteCmd(reg);
    setReadDir();
   _delay_us(1);    //1us should be adequate
    do { ret = read16bits(); }
		while (--index >= 0);  //need to test with SSD1963
    RD_IDLE;
    CS_IDLE;
    //setWriteDir();
    return ret;
}

uint32_t readReg32(uint16_t reg)
{
    uint16_t h = readReg(reg, 0);
    uint16_t l = readReg(reg, 1);
    return ((uint32_t) h << 16) | (l);
}


uint32_t readReg40(uint16_t reg)
{
    uint16_t h = readReg(reg, 0);
    uint16_t m = readReg(reg, 1);
    uint16_t l = readReg(reg, 2);
    return ((uint32_t) h << 24) | (m << 8) | (l >> 8);
}

void tft_init_table(void)
{
			init_table16(ILI9325_regValues, sizeof(ILI9325_regValues));


}
uint16_t tft_readID(void)
{
    uint16_t ret, ret2;
 
    uint8_t msb;
    ret = readReg(0,0);           //forces a reset() if called before begin()
//	printf("ret: %x\n",ret);
    msb = ret >> 8;
    if (msb == 0x93 || msb == 0x94 || msb == 0x98 || msb == 0x77 || msb == 0x16){
	
	//	printf("ret1: %d\n",ret);

        return ret;             //0x9488, 9486, 9340, 9341, 7796

	}


   
	return  readReg(0,0);        //0154, 7783, 9320, 9325, 9335, B505, B509
}


 // independent cursor and window registers.   S6D0154, ST7781 increments.  ILI92320/5 do not.  
int16_t readGRAM(int16_t x, int16_t y, uint16_t * block, int16_t w, int16_t h)
{

    uint16_t ret, dummy, _MR = _MW;
    int16_t n = w * h, row = 0, col = 0;
    uint8_t r, g, b, tmp;
    if (!is8347 && _lcd_capable & MIPI_DCS_REV1) // HX8347 uses same register
        _MR = 0x2E;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    while (n > 0) {
        if (!(_lcd_capable & MIPI_DCS_REV1)) {
            WriteCmdData(_MC, x + col);
            WriteCmdData(_MP, y + row);
        }
        CS_ACTIVE;
        WriteCmd(_MR);
        setReadDir();
        CD_DATA;
        if (_lcd_capable & READ_NODUMMY) {
            ;
        } else if ((_lcd_capable & MIPI_DCS_REV1) || _lcd_ID == 0x1289) {
            READ_8(r);
        } else {
            READ_16(dummy);
        }
		if (_lcd_ID == 0x1511) READ_8(r);   //extra dummy for R61511
        while (n) {
            if (_lcd_capable & READ_24BITS) {
                READ_8(r);
                READ_8(g);
                READ_8(b);
                if (_lcd_capable & READ_BGR)
                    ret = color565(b, g, r);
                else
                    ret = color565(r, g, b);
            } else {
                READ_16(ret);
                if (_lcd_capable & READ_LOWHIGH)
                    ret = (ret >> 8) | (ret << 8);
                if (_lcd_capable & READ_BGR)
                    ret = (ret & 0x07E0) | (ret >> 11) | (ret << 11);

            }
#if defined(SUPPORT_9488_555)
    if (is555) ret = color555_to_565(ret);
#endif
            *block++ = ret;
            n--;
            if (!(_lcd_capable & AUTO_READINC))
                break;
        }
        if (++col >= w) {
            col = 0;
            if (++row >= h)
                row = 0;
        }
        RD_IDLE;
        CS_IDLE;
        setWriteDir();
    }
    if (!(_lcd_capable & MIPI_DCS_REV1))
        setAddrWindow(0, 0, width() - 1, height() - 1);

    return 0;
}



void begin(uint16_t ID)
{
 
    int16_t *p16;               //so we can "write" to a const protected variable.
    const uint8_t *table8_ads = NULL;
    int16_t table_size;
    //gjureset();
    tft_reset();
    _lcd_xor = 0;
//	printf("ID:%x\n",ID);
    _lcd_capable = 0 | REV_SCREEN;





       
	   
	   init_table16(ILI9325_regValues, sizeof(ILI9325_regValues));
 
        p16 = (int16_t *) & WIDTH;
        *p16 = 0;       //error value for WIDTH


/*
    switch (_lcd_ID = ID) {



	case 0x9331:
    case 0x9335:
        _lcd_capable = 0 | REV_SCREEN;
      common_93x5:


        init_table16(ILI9325_regValues, sizeof(ILI9325_regValues));
        break;

 


    default:
        p16 = (int16_t *) & WIDTH;
        *p16 = 0;       //error value for WIDTH
        break;
    }
	*/
    _lcd_rev = ((_lcd_capable & REV_SCREEN) != 0);
    
	if (table8_ads != NULL) {
        static const uint8_t reset_off[] PROGMEM = {
            0x01, 0,            //Soft Reset
            TFTLCD_DELAY8, 150,  // .kbv will power up with ONLY reset, sleep out, display on
            0x28, 0,            //Display Off
            0x3A, 1, 0x55,      //Pixel read=565, write=565.
        };
        static const uint8_t wake_on[] PROGMEM = {
			0x11, 0,            //Sleep Out
            TFTLCD_DELAY8, 150,
            ILI9341_CMD_DISPLAY_ON /*0x29*/, 0,            //Display On
						
        };
		init_table16(&reset_off, sizeof(reset_off));
	    init_table16(table8_ads, table_size);   //can change PIXFMT
		init_table16(&wake_on, sizeof(wake_on));
    }

	WIDTH=240;
	HEIGHT=320;

	setRotation(0);             //PORTRAIT
    invertDisplay(false);


}

void setTextColor(uint16_t c,uint16_t b) 
{
  textcolor = c;
  textbgcolor = b; 
  // for 'transparent' background, we'll set the bg 
  // to the same as fg instead of using a flag
}

void setTextColorWithBgn(uint16_t c, uint16_t b)
{
   textcolor = c;
   textbgcolor = b; 
} 

uint8_t print(const char *buffer, uint8_t size )
{
	//printf("buf:%ssize:%d",buffer,size);
  uint8_t n = 0;
  while (size--) {
    if (write(*buffer++))
	 	n++;
    else 
		break;
  }
  return n;
} 


void text(const char * text, int16_t x, int16_t y) 
{
  if (!useStroke)
    return;
  
//gju  setTextWrap(false);
  setTextWrap(true);
  setTextColor(strokeColor,textbgcolor);
  setCursor(x, y);
  print(text,strlen(text));
} 







void setTextSize(uint8_t s) 
{
  textsize = (s > 0) ? s : 1;
} 
uint8_t write(uint8_t c)
{
  if (c == '\n') {

   	cursor_y += textsize*8;
   	cursor_x = 0;


  } else if (c == '\r') {
    // skip em
  } else {

    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    
	
	//printf("x:%dy:%dc:%d\n",cursor_x, cursor_y,c);//gju wichtig

    //printf("x:%dy:%dc:%dtxt_colot%dtxt_bgn%dsize:%d\n",cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
	cursor_x += textsize*6;
    
	if (wrap && (cursor_x > (_width - textsize*6))) {
      cursor_y += textsize*8;
      cursor_x = 0;
    }


  }
//#if ARDUINO >= 100
  return 1;
//#endif
} 




void setCursor(uint16_t x,uint16_t y)//set cursor at desired location to print data
{
	cursor_x = x;
  	cursor_y = y; 
 }

// draw a character
void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) 
{

  if((x >= _width)		|| // Clip right
     (y >= _height)		|| // Clip bottom
     ((x + 6 * size - 1) < 0)	|| // Clip left
     ((y + 8 * size - 1) < 0))     // Clip top
    return;

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else {  // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
        } 
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else {  // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        } 	
      }
      line >>= 1;
    }
  }
  
} 





//####################################################################################################################################################################################
//                                                                   ILI9341_DisplayONorOFF()
//
// Regelt das ein- oder auschalten des Displays.
//
//                               Variablen
// @ ONorOFF  : Variable fürs ein- oder auschalten des Displays; TRUE für an - FALSE für aus
//
void ILI9341_DisplayONorOFF(char ONorOFF) 
{
  if(ONorOFF == true)
	WriteCmdData(ILI9341_CMD_DISPLAY_OFF,0);                     // Display Anschalten
  else 
	WriteCmdData(ILI9341_CMD_DISPLAY_ON,0);                     // Display Anschalten
}

void backuplocationvset(void)//backing up vset data start location to print next vset data in exact location
{
vsetx=cursor_x;
vsety=cursor_y;
}


void backuplocationvactual(void)//backing up vactual data start location to print next vactual data in exact location
{
vactualx=cursor_x;
vactualy=cursor_y;
}

void backuplocationiset(void)//backing up iset data start location to print next iset data in exact location
{
isetx=cursor_x;
isety=cursor_y;
}


void backuplocationiactual(void)//backing up iactual data start location to print next iactual data in exact location
{
iactualx=cursor_x;
iactualy=cursor_y;
}
void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
  // Update in subclasses if desired!
  drawLine(x, y, x, y+h-1, color);
}


void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) 
{
  // Update in subclasses if desired!
  drawLine(x, y, x+w-1, y, color);
} 

// Bresenham's algorithm - thx wikpedia
void drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1, uint16_t color) 
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}



/*
void drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1,uint16_t color) 
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
} 

*/
// Draw a triangle
void drawTriangle(int16_t x0, int16_t y0,
				int16_t x1, int16_t y1, 
				int16_t x2, int16_t y2, uint16_t color) 
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
} 
// Fill a triangle
void fillTriangle ( int16_t x0, int16_t y0,
				  int16_t x1, int16_t y1,
				  int16_t x2, int16_t y2, uint16_t color)
{
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1,
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }
} 
// Draw a rectangle
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
} 
// draw a circle outline
void drawCircle(int16_t x0, int16_t y0, int16_t r,uint16_t color) 
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);   
  }
} 
void setTextWrap(bool w) 
{
  wrap = w;
} 
uint8_t getRotation(void) 
{
  return rotation;
} 
