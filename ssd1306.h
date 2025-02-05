/**
    @file ssd1306.h
    @brief SSD1306 OLED display driver
    @author David Schramm
    @contributor Turi Scandurra
    @date 2021
    Simple driver for ssd1306 displays
*/

#ifndef _inc_ssd1306
#define _inc_ssd1306
#include <pico/stdlib.h>
#include <hardware/i2c.h>

/**
*	@brief defines commands used in ssd1306
*/
typedef enum {
    SET_COMMAND_MODE = 0x00,
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_DISP_ON = 0xAF,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22,
    SET_DISP_START_LINE = 0x40,
    SET_SEG_REMAP = 0xA0,
    SET_MUX_RATIO = 0xA8,
    SET_COM_OUT_DIR = 0xC0,
    SET_DISP_OFFSET = 0xD3,
    SET_COM_PIN_CFG = 0xDA,
    SET_DISP_CLK_DIV = 0xD5,
    SET_PRECHARGE = 0xD9,
    SET_VCOM_DESEL = 0xDB,
    SET_CHARGE_PUMP = 0x8D
} ssd1306_command_t;

/**
*	@brief holds the configuration
*/
typedef struct {
    uint8_t width; 	/**< width of display */
    uint8_t height; 	/**< height of display */
    uint8_t pages;	/**< stores pages of display (calculated on initialization*/
    uint8_t address; 	/**< i2c address of display*/
    i2c_inst_t *i2c_i; 	/**< i2c connection instance */
    bool external_vcc; 	/**< whether display uses external vcc */ 
    uint8_t *buffer;	/**< display buffer */
    size_t bufsize;	/**< buffer size */
    uint8_t rotation;	/**< display rotation */
} ssd1306_t;

/**
*	@brief initialize display
*
*	@param p : pointer to instance of ssd1306_t
*	@param width : width of display
*	@param height : heigth of display
*	@param address : i2c address of display
*	@param i2c_instance : instance of i2c connection
*	
* 	@return bool.
*	@retval true for Success
*	@retval false if initialization failed
*/
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance);

/**
*	@brief deinitialize display
*
*	@param p : instance of display
*
*/
void ssd1306_deinit(ssd1306_t *p);

/**
*	@brief turn off display
*
*	@param p : instance of display
*
*/
void ssd1306_poweroff(ssd1306_t *p);

/**
	@brief turn on display

	@param p : instance of display

*/
void ssd1306_poweron(ssd1306_t *p);

/**
	@brief set contrast of display

	@param p : instance of display
	@param val : contrast

*/
void ssd1306_contrast(ssd1306_t *p, uint8_t val);

/**
	@brief set invert display

	@param p : instance of display
	@param inv : inv==0: disable inverting, inv!=0: invert

*/
void ssd1306_invert(ssd1306_t *p, uint8_t inv);

/**
	@brief set vertical flip of display

	@param p : instance of display
	@param val : val==0: disable flip, val!=0: flip

*/
void ssd1306_vflip(ssd1306_t *p, uint8_t val);

/**
	@brief set horizontal flop of display

	@param p : instance of display
	@param val : val==0: disable flop, val!=0: flop

*/
void ssd1306_hflip(ssd1306_t *p, uint8_t val);

/**
	@brief Outdated: set hardware rotation of display to 180°

	@param p : instance of display
	@param val : val==0: disable rotation, val!=0: rotate
	@note Included for backwards support. Use ssd1306_set_rotation instead

*/
void ssd1306_rotate(ssd1306_t *p, uint8_t val);

/**
	@brief set display rotation

	@param p : instance of display
	@param rotation : 0: no rotation, 1: 90°, 2: 180°, 3: 270°

*/
void ssd1306_set_rotation(ssd1306_t *p, uint8_t rotation);

/**
	@brief display buffer, should be called on change

	@param p : instance of display

*/
void ssd1306_show(ssd1306_t *p);

/**
	@brief clear display buffer

	@param p : instance of display

*/
void ssd1306_clear(ssd1306_t *p);

/**
	@brief clear pixel on buffer

	@param p : instance of display
	@param x : x position
	@param y : y position
*/
void ssd1306_clear_pixel(ssd1306_t *p, uint32_t x, uint32_t y);

/**
	@brief reset display

	@param p : instance of display
*/
void ssd1306_reset(ssd1306_t *p);

/**
	@brief draw pixel on buffer

	@param p : instance of display
	@param x : x position
	@param y : y position
*/
void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y);

/**
	@brief draw line on buffer

	@param p : instance of display
	@param x1 : x position of starting point
	@param y1 : y position of starting point
	@param x2 : x position of end point
	@param y2 : y position of end point
*/
void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/**
	@brief clear square at given position with given size

	@param p : instance of display
	@param x : x position of starting point
	@param y : y position of starting point
	@param width : width of square
	@param height : height of square
*/
void ssd1306_clear_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
	@brief draw filled square at given position with given size

	@param p : instance of display
	@param x : x position of starting point
	@param y : y position of starting point
	@param width : width of square
	@param height : height of square
*/
void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
	@brief draw empty square at given position with given size

	@param p : instance of display
	@param x : x position of starting point
	@param y : y position of starting point
	@param width : width of square
	@param height : height of square
*/
void ssd1306_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
	@brief clear filled circle at given position with given radius

	@param p : instance of display
	@param x : x position of the center of the circle
	@param y : y position of the center of the circle
	@param r : radius of the circle
*/
void ssd1306_clear_circle(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t r);

/**
	@brief draw filled circle at given position with given radius

	@param p : instance of display
	@param x : x position of the center of the circle
	@param y : y position of the center of the circle
	@param r : radius of the circle
*/
void ssd1306_draw_circle(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t r);

/**
	@brief draw monochrome bitmap with offset

	@param p : instance of display
	@param data : image data (whole file)
	@param size : size of image data in bytes
	@param x_offset : offset of horizontal coordinate
	@param y_offset : offset of vertical coordinate
*/
void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset);

/**
	@brief draw monochrome bitmap

	@param p : instance of display
	@param data : image data (whole file)
	@param size : size of image data in bytes
*/
void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size);

/**
	@brief draw char with given font

	@param p : instance of display
	@param x : x starting position of char
	@param y : y starting position of char
	@param scale : scale font to n times of original size (default should be 1)
	@param font : pointer to font
	@param c : character to draw
*/
void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c);

/**
	@brief draw char with builtin font

	@param p : instance of display
	@param x : x starting position of char
	@param y : y starting position of char
	@param scale : scale font to n times of original size (default should be 1)
	@param c : character to draw
*/
void ssd1306_draw_char(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, char c);

/**
	@brief draw string with given font

	@param p : instance of display
	@param x : x starting position of text
	@param y : y starting position of text
	@param scale : scale font to n times of original size (default should be 1)
	@param font : pointer to font
	@param s : text to draw
*/
void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s );

/**
	@brief draw string with builtin font

	@param p : instance of display
	@param x : x starting position of text
	@param y : y starting position of text
	@param scale : scale font to n times of original size (default should be 1)
	@param s : text to draw
*/
void ssd1306_draw_string(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s);

#endif
