/*
 * gju_ili9341_clock.h
 *
 * Created: 27.02.2020 10:48:32
 *  Author: gju
 */ 



#ifndef GJU_ILI9341_CLOCK_H_
#define GJU_ILI9341_CLOCK_H_

//static uint8_t conv2d(const char* p); // Forward declaration needed for IDE 1.6.x
 //uint8_t conv2d(const char* p); // Forward declaration needed for IDE 1.6.x
//void create_circle();
static uint8_t conv2d(const char* p);
void reverse(char s[]);
void conv_itoa(int n, char s[]);
void create_time_string(void);
void create_date_string(void);
void Draw_clock_face();
void refresh_time();
void refresh_date();
void show_time();
void show_date();
#endif /* GJU_ILI9341_CLOCK_H_ */