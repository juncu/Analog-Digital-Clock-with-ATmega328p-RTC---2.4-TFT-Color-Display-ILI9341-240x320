
//static void writecmddata(uint16_t cmd, uint16_t dat);
//void WriteCmdData(uint16_t cmd, uint16_t dat);                 // ?public methods !!! 

//void pinMode(uint8_t, uint8_t); 
/*
extern const uint16_t PROGMEM port_to_mode_PGM[]; 
extern const uint16_t PROGMEM port_to_input_PGM[];
extern const uint16_t PROGMEM port_to_output_PGM[];
extern const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[]; 

static FILE mydata = FDEV_SETUP_STREAM(ili9341_putchar_printf, NULL, _FDEV_SETUP_WRITE);//mydata declaration and converting it into stream
*/
static const char PROGMEM msg_PGM[]; 
void print_text_landscape_rev(char *msg);
void setCursorPos(uint8_t x,uint8_t y);
void drawCircle(int16_t x0, int16_t y0, int16_t r,uint16_t color); 
