////////////////           TM1628驱动库的应用库      V0.6       //////////////
////////////////     Copyright 2016-2018      by changser       //////////////
////////////////           changser@139.com                     //////////////
 
#ifndef __DIS_KEY_H__
#define __DIS_KEY_H__

#include  "driver_tm1628.h"

////***************提供给外部的接口***************************///////////
//1、提供给外部的宏////////////////////////////////////
//定义小数点位置常数（某数如987654321，从个位为1开始数。如果是2，则98765432.1。跟显示器序号无关）
#define NO_DOT 0
#define DOT   0XFF  //显示一位LED时，需加小数点则用非零DOT来表示。
#define DOT1  1
#define DOT2  2
#define DOT3  3
#define DOT4  4
#define DOT5  5
#define DOT6  6
#define DOT7  7
#define DOT8  8
#define DOT9  9
//定义七段LED位置常数（物理位置，从左到右1~9。映射前的。跟寄存器无关）
#define LED1  1
#define LED2  2
#define LED3  3
#define LED4  4
#define LED5  5
#define LED6  6
#define LED7  7
#define LED8  8
#define LED9  9

//定义按键物理状态，非闭合、闭合。
enum enum_key_state {KEY_OPEN = 0x00, KEY_CLOSE = 0x55};
//定义按键4种动作：没按（松开保持）、按下、按下保持、松开
typedef enum {NO_PRESS = 0x00, KEY_DOWN = 0x55, KEY_KEEP = 0xff, KEY_UP = 0xaa} enum_keyact;
//定义按键结构体
typedef struct
{
  enum_keyact key_buffer;
  uint16_t key_time;
} struct_key;

//定义指示灯的序号，1~14及所有
typedef enum {LAMP1 = 1, LAMP2 = 2, LAMP3 = 3, LAMP4 = 4, LAMP5 = 5, LAMP6 = 6, LAMP7 = 7, LAMP8 = 8,
              LAMP9 = 9, LAMP10 = 10, LAMP11 = 11, LAMP12 = 12, LAMP13 = 13, LAMP14 = 14, LAMP_ALL = 0XFF
             } enum_LAMP;

class APP_TM1628: public TM1628 {
  public:
    APP_TM1628(uint8_t stb , uint8_t dio , uint8_t clk );
    struct_key keys[20]; //键状态数组，存的是按键号对应的按键状态。按键号=数组下标+1
    //接上，在析构函数中和set_key_table()中，动态数组没搞对，先搞固定的。
    //struct_key *keys;
    //若更改物理七段LED顺序，设置新的映射表，缺省的逻辑LED显示器顺序（没有用表）无效。
    void set_dis_table(uint8_t const *p);
    //若更改物理按键的键序，设置新的映射表，缺省的逻辑按键顺序（没有用表）无效。
    void set_key_table(uint8_t const *p, uint8_t keynum);

	//将所有显示器设置为显示8.即字段全亮，包括指示灯.
    void to_led_all8();
    //将某几个显示器设置为显示空，包括指示灯
    void to_led_clear(uint8_t start_led, uint8_t led_long);
    //显示器闪一下，即全点亮1s，并全清空1s
    void to_led_flash();

    //将SEG码直接在TM1628驱动下显示出。（高位->）dp,gfedcba(<-0位)
    void to_led_seg(int8_t seg, uint8_t No);

    //将ASCII对应的单个字符在TM1628驱动下显示出来。可选是否加小数点。
    void to_led_char(int8_t ch , uint8_t dot, uint8_t No  );

    //16位整数数字作为10进制字符串显示。
    uint8_t to_led(int16_t number, uint8_t dot = NO_DOT, uint8_t No = 4 , uint8_t len = 4);
    //32位整数数字作为10进制字符串显示。
    uint8_t to_led(int32_t number, uint8_t dot = NO_DOT, uint8_t No = 4 , uint8_t len = 4);
    //浮点数显示。ARDUINO AVR中，双精度浮点double与单精度float相同，都占4字节。
    uint8_t to_led(double val, uint8_t dot = DOT1, uint8_t No = 4 , uint8_t  len = 4);

    //置位某指示灯，或所有指示灯，只是类变量更改，没涮新硬件
    void set_lamp(enum_LAMP lamp_No);
    //清除某指示灯，或所有指示灯，只是类变量更改，没涮新硬件
    void clr_lamp(enum_LAMP lamp_No);
    //涮新TM1628奇地址寄存器，实现显示。两个共阳七段LED或14个灯一起涮新
    void to_led_lamps(void);

    //功能：取键子函数，结果存放于键盘缓冲区中keys[20]中，对键的四个动作都有描述
    //        keys[i]的下标i=按键序号-1
    //        如果存在按键映射表，只处理表中的几个键，
    //       需被周期性地被调用。不需要消抖（消抖由芯片完成）。
    uint8_t get_key(void);
    //功能：按键操作将某数进行加减的子程序。根据按键时长进行连续加减、加速加减、十倍百倍加减。
    //      注意：key_No是对应的物理键号，是从1开始。
    uint16_t key_add_or_sub(uint8_t key_No, uint16_t variable,
                                uint16_t max_or_min, uint8_t ISadd,
                                uint8_t loop);
  private:
    //定义物理LED显示器与逻辑LED显示器的映射表
    uint8_t const *dis_table_P = NULL;
    //定义两个共阳七段LED，因为对应TM1628的奇地址显示寄存器，且是SEG1-GRID1~7及SEG2-GRID1~7，
    //并且要按位操作（指示灯操作），所以更改某些值不能影响其它值，所以需保存，与显示寄存器同步。
    uint16_t twoCA = 0;
    //由ASCII码查八段码，查结构数组char_array1[]。
    uint8_t char_to_seg(char val);
    //查表，七段LED物理位置序号转逻辑LED位置序号
    uint8_t ledNo_to_ledLogNo(uint8_t ledNo);

    //定义物理按键与按键寄存器的映射表
    uint8_t const *key_table_P = NULL;
    uint8_t key_num = 20;   //按键个数
};

#endif

