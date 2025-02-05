/**
    @file ssd1306.c
    @brief SSD1306 OLED display driver
    @author David Schramm
    @contributor Turi Scandurra
    @date 2021
    Simple driver for ssd1306 displays
*/
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ssd1306.h"
#include "font.h"

/**
 * @brief swap the values of two integers
 *
 * @param a : pointer to the first integer
 * @param b : pointer to the second integer
 */
inline static void swap(int32_t *a, int32_t *b) {
    int32_t *t=a;
    *a=*b;
    *b=*t;
}

/**
 * @brief write data to an I2C device with error handling
 *
 * @param i2c : pointer to the I2C instance
 * @param addr : address of the I2C device
 * @param src : pointer to the data to be written
 * @param len : length of the data to be written
 * @param name : name of the I2C device (for error messages)
 */
inline static void fancy_write(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, char *name) {
    switch(i2c_write_blocking(i2c, addr, src, len, false)) {
    case PICO_ERROR_GENERIC:
        printf("[%s] addr not acknowledged!\n", name);
        break;
    case PICO_ERROR_TIMEOUT:
        printf("[%s] timeout!\n", name);
        break;
    default:
        printf("[%s] wrote successfully %lu bytes!\n", name, len);
        break;
    }
}

/**
 * @brief write a single byte to the display
 *
 * @param p : instance of display
 * @param val : byte to be written
 */
inline static void ssd1306_write(ssd1306_t *p, uint8_t val) {
    uint8_t d[2]= {0x00, val};
    i2c_write_blocking(p->i2c_i, p->address, d, 2, false);
}

/**
*	@brief initialize display
*
*	@param p : instance of display
*	@param width : width of display
*	@param height : heigth of display
*	@param address : i2c address of display
*	@param i2c_instance : instance of i2c connection
*	
* 	@return bool.
*	@retval true for Success
*	@retval false if initialization failed
*/
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance) {
    p->width=width;
    p->height=height;
    p->pages=height/8;
    p->address=address;

    p->i2c_i=i2c_instance;


    p->bufsize=(p->pages)*(p->width);
    if((p->buffer=malloc(p->bufsize+1))==NULL) {
        p->bufsize=0;
        return false;
    }

    ++(p->buffer);

    // from https://github.com/makerportal/rpi-pico-ssd1306
    uint8_t cmds[]= {
        SET_DISP,
        // timing and driving scheme
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        height - 1,
        SET_DISP_OFFSET,
        0x00,
        // resolution and layout
        SET_DISP_START_LINE,
        // charge pump
        SET_CHARGE_PUMP,
        p->external_vcc?0x10:0x14,
        SET_SEG_REMAP | 0x01,           // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08,         // scan from COM[N] to COM0
        SET_COM_PIN_CFG,
        width>2*height?0x02:0x12,
        // display
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        p->external_vcc?0x22:0xF1,
        SET_VCOM_DESEL,
        0x30,                           // or 0x40?
        SET_ENTIRE_ON,                  // output follows RAM contents
        SET_NORM_INV,                   // not inverted
        SET_DISP | 0x01,
        // address setting
        SET_MEM_ADDR,
        0x00,  // horizontal
    };

    for(size_t i=0; i<sizeof(cmds); ++i)
        ssd1306_write(p, cmds[i]);

    return true;
}

/**
*	@brief deinitialize display
*
*	@param p : instance of display
*
*/
inline void ssd1306_deinit(ssd1306_t *p) {
    free(p->buffer-1);
}

/**
*	@brief turn off display
*
*	@param p : instance of display
*
*/
inline void ssd1306_poweroff(ssd1306_t *p) {
    ssd1306_write(p, SET_DISP|0x00);
}

/**
	@brief turn on display

	@param p : instance of display

*/
inline void ssd1306_poweron(ssd1306_t *p) {
    ssd1306_write(p, SET_DISP|0x01);
}

/**
	@brief set contrast of display

	@param p : instance of display
	@param val : contrast

*/
inline void ssd1306_contrast(ssd1306_t *p, uint8_t val) {
    ssd1306_write(p, SET_CONTRAST);
    ssd1306_write(p, val);
}

/**
	@brief set invert display

	@param p : instance of display
	@param inv : inv==0: disable inverting, inv!=0: invert

*/
inline void ssd1306_invert(ssd1306_t *p, uint8_t inv) {
    ssd1306_write(p, SET_NORM_INV | (inv & 1));
}

/**
	@brief set vertical flip of display

	@param p : instance of display
	@param val : val==0: disable flip, val!=0: flip

*/
inline void ssd1306_vflip(ssd1306_t *p, uint8_t val) {
    ssd1306_write(p, SET_COM_OUT_DIR | (!val << 3));
}

/**
	@brief set horizontal flop of display

	@param p : instance of display
	@param val : val==0: disable flop, val!=0: flop

*/
inline void ssd1306_hflip(ssd1306_t *p, uint8_t val) {
    ssd1306_write(p, SET_SEG_REMAP | (!val & 1));
}

/**
	@brief Outdated: set hardware rotation of display to 180°

	@param p : instance of display
	@param val : val==0: disable rotation, val!=0: rotate
	@note Included for backwards support. Use ssd1306_set_rotation instead

*/
inline void ssd1306_rotate(ssd1306_t *p, uint8_t val) {
    ssd1306_vflip(p, val);
    ssd1306_hflip(p, val);
}

/**
	@brief set display rotation

	@param p : instance of display
	@param rotation : 0: no rotation, 1: 90°, 2: 180°, 3: 270°

*/
inline void ssd1306_set_rotation(ssd1306_t *p, uint8_t rotation) {
    if(rotation > 3) {
        return;
    }

    p->rotation = rotation;
}

/**
	@brief clear display buffer

	@param p : instance of display

*/
inline void ssd1306_clear(ssd1306_t *p) {
    memset(p->buffer, 0, p->bufsize);
}

/**
	@brief clear pixel on buffer

	@param p : instance of display
	@param x : x position
	@param y : y position
*/
void ssd1306_clear_pixel(ssd1306_t *p, uint32_t x, uint32_t y) {
    uint32_t buffer_x, buffer_y;
    switch (p->rotation) {
        case 0:
            buffer_x = x;
            buffer_y = y;
            break;
        case 1:
            buffer_x = p->height - 1 - y + (p->width) / 2;
            buffer_y = x;
            break;
        case 2:
            buffer_x = p->width - 1 - x;
            buffer_y = p->height - 1 - y;
            break;
        case 3:
            buffer_x = y;
            buffer_y = p->height - 1 - x;
            break;
    }

    if (buffer_x < p->width && buffer_y < p->height) {
        p->buffer[buffer_x + p->width * (buffer_y >> 3)] &= ~(0x1 << (buffer_y & 0x07));
    }
}

/**
	@brief reset display

	@param p : instance of display
*/
void ssd1306_reset(ssd1306_t *p) {
    uint8_t payload[] = {
        SET_COMMAND_MODE,
        SET_DISP,
        SET_ENTIRE_ON,
        SET_DISP_CLK_DIV,
        0x80,
        SET_CHARGE_PUMP,
        0x14,
        SET_NORM_INV,
        SET_DISP_OFFSET,
        0x00,
        SET_DISP_START_LINE,
        SET_DISP_ON
    };

    for (size_t i = 0; i < sizeof(payload); ++i)
        ssd1306_write(p, payload[i]);
}

/**
	@brief draw pixel on buffer

	@param p : instance of display
	@param x : x position
	@param y : y position
*/
void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y) {
    uint32_t buffer_x, buffer_y;
    switch (p->rotation) {
        case 0:
            buffer_x = x;
            buffer_y = y;
            break;
        case 1:
            buffer_x = p->height - 1 - y + p->width / 2;
            buffer_y = x;
            break;
        case 2:
            buffer_x = p->width - 1 - x;
            buffer_y = p->height - 1 - y;
            break;
        case 3:
            buffer_x = y;
            buffer_y = p->height - 1 - x;
            break;
    }

    if (buffer_x < p->width && buffer_y < p->height) {
        p->buffer[buffer_x + p->width * (buffer_y >> 3)] |= 0x1 << (buffer_y & 0x07);
    }
}

/**
	@brief draw line on buffer

	@param p : instance of display
	@param x1 : x position of starting point
	@param y1 : y position of starting point
	@param x2 : x position of end point
	@param y2 : y position of end point
*/
void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1>x2) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }

    if(x1==x2) {
        if(y1>y2)
            swap(&y1, &y2);
        for(int32_t i=y1; i<=y2; ++i)
            ssd1306_draw_pixel(p, x1, i);
        return;
    }

    float m=(float) (y2-y1) / (float) (x2-x1);

    for(int32_t i=x1; i<=x2; ++i) {
        float y=m*(float) (i-x1)+(float) y1;
        ssd1306_draw_pixel(p, i, (uint32_t) y);
    }
}

/**
	@brief clear square at given position with given size

	@param p : instance of display
	@param x : x position of starting point
	@param y : y position of starting point
	@param width : width of square
	@param height : height of square
*/
void ssd1306_clear_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    for(uint32_t i=0; i<width; ++i)
        for(uint32_t j=0; j<height; ++j)
            ssd1306_clear_pixel(p, x+i, y+j);
}

/**
	@brief draw filled square at given position with given size

	@param p : instance of display
	@param x : x position of starting point
	@param y : y position of starting point
	@param width : width of square
	@param height : height of square
*/
void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    for(uint32_t i=0; i<width; ++i)
        for(uint32_t j=0; j<height; ++j)
            ssd1306_draw_pixel(p, x+i, y+j);
}

/**
	@brief draw empty square at given position with given size

	@param p : instance of display
	@param x : x position of starting point
	@param y : y position of starting point
	@param width : width of square
	@param height : height of square
*/
void ssd1306_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    ssd1306_draw_line(p, x, y, x+width, y);
    ssd1306_draw_line(p, x, y+height, x+width, y+height);
    ssd1306_draw_line(p, x, y, x, y+height);
    ssd1306_draw_line(p, x+width, y, x+width, y+height);
}

/**
	@brief clear filled circle at given position with given radius

	@param p : instance of display
	@param x : x position of the center of the circle
	@param y : y position of the center of the circle
	@param r : radius of the circle
*/
void ssd1306_clear_circle(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t r) {
    for (uint32_t i = 0; i < r; ++i) {
        for (uint32_t j = 0; j < r; ++j) {
            if ((i * i + j * j) <= (r * r)) {
                ssd1306_clear_pixel(p, x + i, y + j);
                ssd1306_clear_pixel(p, x + i, y - j);
                ssd1306_clear_pixel(p, x - i, y + j);
                ssd1306_clear_pixel(p, x - i, y - j);
            }
        }
    }
}

/**
	@brief draw filled circle at given position with given radius

	@param p : instance of display
	@param x : x position of the center of the circle
	@param y : y position of the center of the circle
	@param r : radius of the circle
*/
void ssd1306_draw_circle(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t r) {
    for (uint32_t i = 0; i < r; ++i) {
        for (uint32_t j = 0; j < r; ++j) {
            if ((i * i + j * j) <= (r * r)) {
                ssd1306_draw_pixel(p, x + i, y + j);
                ssd1306_draw_pixel(p, x + i, y - j);
                ssd1306_draw_pixel(p, x - i, y + j);
                ssd1306_draw_pixel(p, x - i, y - j);
            }
        }
    }
}

/**
	@brief draw char with given font

	@param p : instance of display
	@param x : x starting position of char
	@param y : y starting position of char
	@param scale : scale font to n times of original size (default should be 1)
	@param font : pointer to font
	@param c : character to draw
*/
void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c) {
    if(c<font[3]||c>font[4])
        return;

    uint32_t parts_per_line=(font[0]>>3)+((font[0]&7)>0);
    for(uint8_t w=0; w<font[1]; ++w) { // width
        uint32_t pp=(c-font[3])*font[1]*parts_per_line+w*parts_per_line+5;
        for(uint32_t lp=0; lp<parts_per_line; ++lp) {
            uint8_t line=font[pp];

            for(int8_t j=0; j<8; ++j, line>>=1) {
                if(line & 1)
                    ssd1306_draw_square(p, x+w*scale, y+((lp<<3)+j)*scale, scale, scale);
            }

            ++pp;
        }
    }
}

/**
	@brief draw string with given font

	@param p : instance of display
	@param x : x starting position of text
	@param y : y starting position of text
	@param scale : scale font to n times of original size (default should be 1)
	@param font : pointer to font
	@param s : text to draw
*/
void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s) {
    for(int32_t x_n=x; *s; x_n+=(font[1]+font[2])*scale) {
        ssd1306_draw_char_with_font(p, x_n, y, scale, font, *(s++));
    }
}

/**
	@brief draw char with builtin font

	@param p : instance of display
	@param x : x starting position of char
	@param y : y starting position of char
	@param scale : scale font to n times of original size (default should be 1)
	@param c : character to draw
*/
void ssd1306_draw_char(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, char c) {
    ssd1306_draw_char_with_font(p, x, y, scale, font_8x5, c);
}

/**
	@brief draw string with builtin font

	@param p : instance of display
	@param x : x starting position of text
	@param y : y starting position of text
	@param scale : scale font to n times of original size (default should be 1)
	@param s : text to draw
*/
void ssd1306_draw_string(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s) {
    ssd1306_draw_string_with_font(p, x, y, scale, font_8x5, s);
}

/**
 * @brief retrieve a value from a BMP image header.
 *
 * @param data : pointer to the BMP image data.
 * @param offset : offset of the value in the header.
 * @param size : size of the value (1, 2, or 4 bytes).
 * @return : the retrieved value.
 */
static inline uint32_t ssd1306_bmp_get_val(const uint8_t *data, const size_t offset, uint8_t size) {
    switch(size) {
    case 1:
        return data[offset];
    case 2:
        return data[offset]|(data[offset+1]<<8);
    case 4:
        return data[offset]|(data[offset+1]<<8)|(data[offset+2]<<16)|(data[offset+3]<<24);
    default:
        __builtin_unreachable();
    }
    __builtin_unreachable();
}

/**
	@brief draw monochrome bitmap with offset

	@param p : instance of display
	@param data : image data (whole file)
	@param size : size of image data in bytes
	@param x_offset : offset of horizontal coordinate
	@param y_offset : offset of vertical coordinate
*/
void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset) {
    if(size<54) // data smaller than header
        return;

    const uint32_t bfOffBits=ssd1306_bmp_get_val(data, 10, 4);
    const uint32_t biSize=ssd1306_bmp_get_val(data, 14, 4);
    const uint32_t biWidth=ssd1306_bmp_get_val(data, 18, 4);
    const int32_t biHeight=(int32_t) ssd1306_bmp_get_val(data, 22, 4);
    const uint16_t biBitCount=(uint16_t) ssd1306_bmp_get_val(data, 28, 2);
    const uint32_t biCompression=ssd1306_bmp_get_val(data, 30, 4);

    if(biBitCount!=1) // image not monochrome
        return;

    if(biCompression!=0) // image compressed
        return;

    const int table_start=14+biSize;
    uint8_t color_val=0;

    for(uint8_t i=0; i<2; ++i) {
        if(!((data[table_start+i*4]<<16)|(data[table_start+i*4+1]<<8)|data[table_start+i*4+2])) {
            color_val=i;
            break;
        }
    }

    uint32_t bytes_per_line=(biWidth/8)+(biWidth&7?1:0);
    if(bytes_per_line&3)
        bytes_per_line=(bytes_per_line^(bytes_per_line&3))+4;

    const uint8_t *img_data=data+bfOffBits;

    int32_t step=biHeight>0?-1:1;
    int32_t border=biHeight>0?-1:-biHeight;

    for(uint32_t y=biHeight>0?biHeight-1:0; y!=(uint32_t)border; y+=step) {
        for(uint32_t x=0; x<biWidth; ++x) {
            if(((img_data[x>>3]>>(7-(x&7)))&1)==color_val)
                ssd1306_draw_pixel(p, x_offset+x, y_offset+y);
        }
        img_data+=bytes_per_line;
    }
}

/**
	@brief draw monochrome bitmap

	@param p : instance of display
	@param data : image data (whole file)
	@param size : size of image data in bytes
*/
inline void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size) {
    ssd1306_bmp_show_image_with_offset(p, data, size, 0, 0);
}

/**
	@brief display buffer, should be called on change

	@param p : instance of display

*/
void ssd1306_show(ssd1306_t *p) {
    uint8_t payload[]= {SET_COL_ADDR, 0, p->width-1, SET_PAGE_ADDR, 0, p->pages-1};
    if(p->width==64) {
        payload[1]+=32;
        payload[2]+=32;
    }

    for(size_t i=0; i<sizeof(payload); ++i)
        ssd1306_write(p, payload[i]);

    *(p->buffer-1)=0x40;

    i2c_write_blocking(p->i2c_i, p->address, p->buffer-1, p->bufsize+1, false);
}
