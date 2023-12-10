////#ifndef __ARDUCAM_H
//#define __ARDUCAM_H
//
//#define byte uint8_t
//
#include "ov5642_regs.h"


#define pgm_read_word(x)        ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))

#define ARDUCHIP_TIM       		0x03  //Timming control

#define HREF_LEVEL_MASK    		0x01  //0 = High active , 		1 = Low active
#define VSYNC_LEVEL_MASK   		0x02  //0 = High active , 		1 = Low active
#define LCD_BKEN_MASK      		0x04  //0 = Enable, 					1 = Disable
#define PCLK_DELAY_MASK  		0x08  //0 = data no delay,		1 = data delayed one PCLK

#define ARDUCHIP_FIFO      		0x04  //FIFO and I2C control
#define FIFO_CLEAR_MASK    		0x01
#define FIFO_START_MASK    		0x02
#define FIFO_RDPTR_RST_MASK     0x10
#define FIFO_WRPTR_RST_MASK     0x20

#define ARDUCHIP_TRIG      		0x41  //Trigger source
#define VSYNC_MASK         		0x01
#define SHUTTER_MASK       		0x02
#define CAP_DONE_MASK      		0x08

#define BURST_FIFO_READ			0x3C
#define SINGLE_FIFO_READ		0x3D
