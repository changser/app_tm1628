////////////////           TM1628的驱动库      V0.5           //////////////
////////////////     Copyright 2016-2017      by changser      //////////////
////////////////           changser@139.com                     ///////////////////////////

#ifndef __DRIVER_TM1628_H__
#define __DRIVER_TM1628_H__

#include <arduino.h>

#define delay_us(x) delayMicroseconds(x)
#define delay_ms(x) delay(x)

#define ON 0XFF
#define OFF 0

typedef enum {GRID4_SEG13 = 0, GRID5_SEG12 = 1, GRID6_SEG11 = 2, GRID7_SEG10 = 3} enum_dismode;

class TM1628 {
  public:
    TM1628(uint8_t stb  , uint8_t dio  , uint8_t clk  ) ; 
    uint8_t key_R[5];             //TM1628有5字节的可读寄存器（只能依次读），支持20个按键
    void set_mode(enum_dismode dismode = GRID7_SEG10);      //设置器件工作模式，缺省7位10段
    void set_onoff(uint8_t onoff);                    //亮（非0）或灭（0），硬件操作，不改变原来的亮度。
    void set_brightness(uint8_t level = 4);           //level:0~7共8级亮度，硬件操作，设置亮度时会同时点亮
    void set_address(uint8_t addr = 0, uint8_t inc = 1);  //设置待操作的显示寄存器的地址并变寄存器可写；设置是否自动加。硬件操作。
    void write_disR(uint8_t data);                    //写显示寄存器，跟在set_address（）后面。本身不包括set_address()内容。
    void read_keyR(void);                                   //读按键寄存器
    void end_cs(void);                                      //结束片选，即stb_pin变高。成员函数不用，直接用语句。提供给外部使用。一般长期不操作器件之前就可以用。
  private:
    uint8_t stb_pin;            //片选脚，高到低时选中
    uint8_t dio_pin;            //数据脚
    uint8_t clk_pin;            //时钟脚
    //enum_dismode mode;          //显示模式。0：4位13段；1：5位12段；2：6位11段；3:7位10段（缺省）//不用保存，在set_mode（）中执行就可以
    uint8_t brightness;         //亮度级别0~7级，7级最亮，需保存，亮或灭功能 set_onoff（）会调用
    //uint8_t dis_adrress;        //硬件显示寄存器地址。可以设置。按键的不能设置。
    //uint8_t *dis_buffer_p;      //指向硬件显示寄存器地址应得到的显示数据。
    void init_tm1628_PORT(void);      //初始化IO口
    void send_8bit(uint8_t dat);
    void command(uint8_t com);
};

#endif

