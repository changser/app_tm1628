1、主题、版权说明
////////////////          TM1628驱动库          V0.5           //////////////
////////////////          TM1628驱动库的应用库  V0.7           //////////////
////////////////     Copyright 2016-2018      by changser      //////////////
////////////////           changser@139.com                    //////////////

2、版本说明
V0.7        20180704              应用库增加key_func_done()功能函数；
                                  增加key_add_or_sub()对调用周期的要求说明
                                  增加test_pt/test_pt1/test_pt2三个例程
V0.6        20180626              应用库、例子TEST、本readme.txt、原理图的更新。
                                  本文件应用说明部分更详细；
                                  dis_key.h/cpp跟按键相关的注释更详细；
                                   key_add_or_sub(）的参数uint8_t key_No由键数组
                                   下标号（从0开始）改为键序号（从1开始）
                                  例子TEST注释部分更多。key_add_or_sub()调用参数变化
                                  原理图更新，明确了显示器序号             
V0.5        20161003~20170226     建立TM1628的驱动库和应用库 ，在IDE1.8.1下,
				  选UNO或mega硬件通过；选M0时to_led()存在重载问题.   

3、文件列表
..\app_tm1628\
             -test\test.ino         示例文件，注意ARDUINO程序文件名需与文件夹同名
             -test_pt\test_pt.ino   基于PT(changser_pt_for_arduino版本V20170611)的示例文件
             -test_pt1\test_pt1.ino 基于PT(changser_pt_for_arduino版本V20170611)的示例文件1
	     -test_pt2\test_pt2.ino 基于PT(changser_pt_for_arduino版本V20170611)的示例文件2
	     -ifs_class_dis_example\ifs_class_dis_example.ino  	示例文件
	     -readme.TXT       	    本文件，说明文档
	     -TM1628键盘显示.schdoc 用芯片TM1628的键盘显示电路原理图
	     -TM1628键盘显示.PDF    PDF格式原理图
	     -driver_tm1628.h       芯片TM1628的驱动库头文件，有类TM1628
	     -driver_tm1628.cpp	    芯片TM1628的驱动库，有类TM1628
	     -dis_key.h		    TM1628的键盘显示应用库头文件，有类APP_TM1628，
			            基于上述电路图和驱动库
	     -dis_key.cpp	    TM1628的键盘显示应用库，有类APP_TM1628
			
4、功能说明
TM1628驱动库：针对芯片TM1628操作，主要是面对寄存器操作
TM1628驱动库的应用库：基于上述TM1628电路图和驱动库的键盘显示应用，主要是应用层面操作，
           面对的是7个共阴8段和2个共阳7段LED显示器（后两个还可以当作14个指示灯，逐位操作）
	   以及最多20个按键。
   
5、应用说明
更详细的参见例程。还可查看类TM1628和APP_TM1628声明与定义
1>包含库头文件（如何加载ARDUINO库见ARDUINO相关文档）：
    #include "dis_key.h"

2>类库实例化，指定硬件管脚。参数是ARDUINO数字IO口号。
   APP_TM1628 app_tm1628 = APP_TM1628(11, 12, 13); //STB=11,DIO=12,CLK=13
   其中app_tm1628是实例名，也可以是别的名字。

3>若物理LED显示器、物理按键与原理图上的排列不符，需定义数组进行映射。
  无论是显示器还是按键，排列都是从1开始排。显示器从左到右1到9，按键从1到20
  static const uint8_t dis_table1[9] = {6, 5, 4 , 3, 2, 1, 7, 8, 9 };  
  app_tm1628.set_dis_table( dis_table1);  //dis_table1 <=> &dis_table1[0]

  static const uint8_t key_table1[] = {1, 4, 5, 8};
  app_tm1628.set_key_table(key_table1, sizeof(key_table1));

4>针对显示器的操作（类的共公函数），调用方法如同前面第3>条app_tm1628.xxx()。可对1~9号显示器
  进行操作，操作方法是一们的，不管是共阴的7个还是共阳的两个（共阳的两个，小数点操作了也无效）
  若存在显示器号的参数，在有映射的情况下，都是物理显示器号，从1~9。
    //针对所有显示器的操作（清除操作可选部分显示器）：
    //将所有显示器设置为显示8.即字段全亮，包括指示灯.
    void to_led_all8();
    //将某几个显示器设置为显示空，包括指示灯
    void to_led_clear(uint8_t start_led, uint8_t led_long);
    //显示器闪一下，即全点亮1s，并全清空1s。包括指示灯
    void to_led_flash();

   //针对单个显示器的操作：
   //将SEG码直接在TM1628驱动下显示出。（高位->）dp,gfedcba(<-0位)
    void to_led_seg(int8_t seg, uint8_t No);
    //将ASCII对应的单个字符在TM1628驱动下显示出来。可选是否加小数点。
    //具体有哪些字符见dis_key.cpp类库的段码表char_array[]
    void to_led_char(int8_t ch , uint8_t dot, uint8_t No  );

    //针对LED屏的显示数字操作，默认是显示器从左到右序号是1~9，显示在定位值及左边，
    //而小数点是从个数为1开始算，如4321，取DOT=2，显示432.1。三条语句如下：
    //16位整数数字作为10进制字符串显示。
    uint8_t to_led(int16_t number, uint8_t dot = NO_DOT, uint8_t No = 4 , uint8_t len = 4);
    //32位整数数字作为10进制字符串显示。
    uint8_t to_led(int32_t number, uint8_t dot = NO_DOT, uint8_t No = 4 , uint8_t len = 4);
    //浮点数显示。ARDUINO AVR中，双精度浮点double与单精度float相同，都占4字节。
    uint8_t to_led(double val, uint8_t dot = DOT1, uint8_t No = 4 , uint8_t  len = 4);

5>针对指示灯的操作：
  a、指示灯有两种，一种就是用逻辑显示器1~7的硬件资源，不过是将整个LED显示器换成多个单个的指示灯。
     针对这种指示灯的操作，可以用上述4>的语句，将段码按自己的意愿一位位排列后操作：
       //将SEG码直接在TM1628驱动下显示出。（高位->）dp,gfedcba(<-0位)
        void to_led_seg(int8_t seg, uint8_t No);
        其中uint8_t No是LED显示器号，如果存在映射，则是对物理LED显示器号进行操作。
  b、还有一种，就是用逻辑显示器8和9的硬件资源，共14个指示灯。当然，也可以是2个共阳显示器（没小数点）
     操作方法有两种，一种是仍用上面的SEG码操作；
     还有专门的三条语句：
       //置位某指示灯，或所有指示灯，只是类变量更改，没涮新硬件
        void set_lamp(enum_LAMP lamp_No);
       //清除某指示灯，或所有指示灯，只是类变量更改，没涮新硬件
        void clr_lamp(enum_LAMP lamp_No);
       //涮新TM1628奇地址寄存器，实现显示。两个共阳七段LED或14个灯一起涮新
        void to_led_lamps(void); 
     上述指示灯号是枚举型的，定义如下，并且跟有没有映射没有关系。
	typedef enum {LAMP1 = 1, LAMP2 = 2, LAMP3 = 3, LAMP4 = 4, LAMP5 = 5, LAMP6 = 6, LAMP7 = 7, LAMP8 = 8,
              LAMP9 = 9, LAMP10 = 10, LAMP11 = 11, LAMP12 = 12, LAMP13 = 13, LAMP14 = 14, LAMP_ALL = 0XFF
             } enum_LAMP;
      其中，LAMP1~7对应着LED9的a~g;LAMP8~14对应着LED8的a~g。

6>还有驱动库driver_tm1628提供的跟显示有关的操作：
	void set_onoff(uint8_t onoff);                    //TM1628芯片级亮（非0）或灭（0），硬件操作，不改变原来的亮度。
    	void set_brightness(uint8_t level = 4);           //level:0~7共8级亮度，硬件操作，设置亮度时会同时点亮
    这两条是driver_tm1628.h/.cpp定义的TM1628库里的，但被APP_TM1628类继承，所以，
    包含了dis_key.h之后，并且实例化APP_TM1628之后，不必针对driver_tm1628做什么，象操作上述实例一样操作就行了，例如：
        app_tm1628.set_onoff(OFF);
    驱动库里的其它语句不建议用户操作。    

7>针对按键的操作：
a、类库提供一个数组和两个功能函数。
    //键状态数组，存的是按键号对应的按键状态。若存在按键映射表，则物理按键号=数组下标+1
    //目前是20个元素的数组，若用按键映射表，只有前key_num（为类私有变量，存的是20或物理按键个数）个元素有用。
    //用户程序中可以读取某键i（从1开始的物理或逻辑键吼）对应的数组keys[i-1]，来进行按键操作
    struct_key keys[20];
    //功能：取键子函数，结果存放于键盘缓冲区中keys[20]中，对键的四个动作都有描述
    //        如果存在按键映射表，只处理表中的几个键，keys[i]中序号i=物理按键序号-1
    //       应放在用户程序loop()之中，时间间隔基本恒定地（比如50ms）不断被调用。
    uint8_t get_key(void);
    //功能：按键操作将某数进行加减的子程序。根据按键时长进行连续加减、加速加减、十倍百倍加减。
    //       key_No是从0开始，对应着按键数组的序号（下标号）。
    //       应用时也是放在用户程序loop()之中，被调用间隔应是20~50ms，配合显示才能对达到按下键有加减1效果。
    uint16_t key_add_or_sub(uint8_t key_No, uint16_t variable,
                                uint16_t max_or_min, uint8_t add,
                                uint8_t loop);
b、数组涉及的定义如下
    struct_key已被定义如下：
     typedef struct   //定义按键结构体
     {
          enum_keyact key_buffer;
          uint16_t key_time; 
      }   struct_key;
     enum_keyact已被定义如下：
         typedef enum {NO_PRESS = 0x00, KEY_DOWN = 0x55, KEY_KEEP = 0xff, KEY_UP = 0xaa} enum_keyact;
      对应着按键4种状态：没按（松开保持）、按下动作、按下保持、松开动作

c、应用时的详细步骤：
     应用时，首先进行按键排序映射，见本应用说明的第3>条。
     然后在loop()中，每固定间隔调用一次get_key():
         app_tm1628.get_key();
         delay_ms(48);
      上面的延时安排，大约是间隔50ms一次。但这种延时是让单片机转圈圈，如果任务多，时间资源紧张，可以考虑协程（见我整理的PT）或操作系统。
      这种操作之后，就可以将每个键的状态即时地取得，存于数组之中，通过读取判断数组状态进行按键操作，如下所下：
         if (app_tm1628.keys[1].key_buffer == KEY_DOWN)  app_tm1628.to_led((int16_t)i, NO_DOT, LED4, 2);
         else if (app_tm1628.keys[1].key_buffer == KEY_UP) app_tm1628.to_led_clear(LED4, 2);
      上述语句是对物理按键2进行判断。keys[0]对应按键1，依次类推。
      如果要将按键时长加入判断，可以如下操作：
         if ((app_tm1628.keys[1].key_buffer == KEY_KEEP)&& (app_tm1628.keys[1].key_time>=100)) xxx
      对于类提供的功能函数 key_add_or_sub()的应用，参见示例。
      