////////////////      APP_TM1628(含TM1628类)类库应用程序例程                          ////////////
////////////////      test   V180626                                                  ////////////
////////////////      by changser    20180626                                         ////////////
////////////////      changser@139.com                                                ////////////

//说明：
//APP_tm1628目录下含两个类库
//driver_tm1628.h和.c中，有类TM1628，是针对键盘显示驱动芯片TM1628的驱动程序，操作目标是器件或者说是寄存器更贴切；
//dis_key.h和.c中，有类APP_TM1628，是针对基于TM1628库的键盘显示电路（见原理图）。操作目标是9个LED显示器（7共阴，2共阳），和20个按键。
//2个共阳的又可以当作灯（14路）来单路或一起仿问
//本例程演示了如何使用类库的功能函数。包括各种基本设置、字段显示、字符显示、整数和浮点显示，以及指示灯操作。
//并且提供了以下实用功能：
//1、对显示器位和段的显示检测和序号识别，若对位顺序不满意可通过映射表来改变。段顺序没法改变；
//2、对按键序号（数组下标）识别，按键序号应+1。若对按键顺序不满意可通过映射表来改变；
//3、将显示和按键联动，实现用按键控制显示亮度，并且不同亮度对应用不同的指示灯来指示。

//更改记录：
//V180626   基于1.8.1，修改注释，主要是对显示器、指示灯和按键，其物理序号、逻辑序号等进行进一步定义，
//           跟原理图进一步对应。
//           因为类中功能函数 key_add_or_sub(uint8_t key_No, uint16_t variable,
//                                uint16_t max_or_min, uint8_t ISadd,
//                                uint8_t loop);
//              的参数 uint8_t key_No，由键数组下标（从0开始）更改为键序号（从1开始），
//           所以相关调用部分进行了更改。                             
//V170319   本文件，基于IDE1.8.1，硬件UNO和M0（未测试）。
//          在MO硬件平台上，易在to_led()处出现 call of XXX overloaded is ambiguous
//          原因是重入的问题。在示例中调用to_led()时，对要显示参数进行明确的类型强制转换。
//V170226   本文件，基于IDE1.8.1
//          用本示例消除多个类的warning，特别是类声明中初始化后，类实现中又初始化的，取逍实现中的初始化；
//             以及TM1628::TM1628中初始化列表顺序与声明不符的问题；
//             以及dis_table1[9]和key_table1[]定义中的const属性，与类里
//             void APP_TM1628::set_dis_table(uint8_t *p)定义不符的问题。更改类定义来消除。
//V161228   建立，基于IDE 1.6.11

//////////////////////////////////////////////////////////////////////////////////////////////////
//用户头文件
#include "dis_key.h"

//变量常量定义：

//设置物理LED显示器映射表。
//物理LED显示器：制板出来的印制板实物上，假如LED显示器是一排的，那么从左到右，物理LED显示器的序号为从1~9。
//               定义这个，主要是进行数字显示时，如4321，根据个位来定位，在定位及左边的LED显示器上显示4321.
//逻辑LED显示器：依原理图器件管脚对LED显示器进行定义。从LED1起从左往右排（原理图上有标注），逻辑LED显示器序号为从1~9。
//缺省状态下（如果不定义此映射表的话），dis_key类库对LED显示器的操作是按逻辑LED显示器定义的序号来操作。
//LED指示灯共14个，也组合成逻辑LED显示器的第8、第9个。原理图中SEG9-GRID1对应第1个指示灯。
//当进行对LED显示器操作涉及逻辑第8、9个时，跟其它显示器一样操作，不过小数点显示不出来；
//当对上述14个指示灯进行操作时(用下面三个语句)，则不管有没有映射，直接操作这14个指示灯：
//    void set_lamp(enum_LAMP lamp_No);  设置单个或全部指示灯缓冲区
//    void clr_lamp(enum_LAMP lamp_No);  清除单个或全部指示灯缓冲区
//    void to_led_lamps(void);           写入到硬件，实现显示
//若定义映射表，数组dis_table1[0]对应物理LED显示器1，存的是逻辑LED显示器序号（1~9）。
//一定要是9个？实际没有的怎么处理？目前类库是按9个处理的。必须填9个。物理不存在的就1~9随便填充，不要重复。
//下面映射表说明：印制板上实物是从左往右为逻辑第6位（原理图上LED6）……第1位（原理图上LED1），
//另外逻辑第7位、8位、第9位
static const uint8_t dis_table1[9] = {6, 5, 4 , 3, 2, 1, 7, 8, 9 };    

//设置物理按键映射表。
//物理按键：板上的实际按键，依自己的意愿定义为1~20（一般也按一定的实际排列顺序），最多可以有20个按键。
//          键状态数组元素keys[0]对应物理按键1，依次类推。
//逻辑按键：原理图上的按键，按K1~K20标的定义为1~20。
//缺省状态下（如果不定义此映射表的话，dis_key类库对按键的操作是按逻辑按键来操作的，且为20个。
//若定义映射表，有几个物理按键就填充几个数组元素。key_table1[0]对应物理按键1，将对应的逻辑按键的序号（1~20）填入。
//下面映射表说明：印制板上实际有4个按键，按键1\2\3\4依次对应原理图上的K1/K4/K5/K8
static const uint8_t key_table1[] = {1, 4, 5, 8};

APP_TM1628 app_tm1628 = APP_TM1628(11, 12, 13); //STB=11,DIO=12,CLK=13

/////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("hello,welcome");
  //若LED显示器排列不是和标准电路相一致，则建立映射，并告知
  app_tm1628.set_dis_table( dis_table1);  //dis_table1 <=> &dis_table1[0]
  //或KEY键排列不是和标准电路相一致，则建立映射，并告知
  app_tm1628.set_key_table(key_table1, sizeof(key_table1));
  //所有显示闪一下（1s），包括指示灯，再都清空
  app_tm1628.to_led_flash();

  //段码方式显示（单个LED操作）:段码排列：dp,gfedcba(低位)，可实现所有显示操作
  //从物理LED序号1~9，每个LED都依次从字段a->dp显示1秒。
  uint8_t i, j;
  for (i = 0; i < 9; i++) //最多可有9个显示器
  {
    app_tm1628.to_led_seg(0, i + 1); //可用该语句将该显示器先清空（单个）
    for (j = 0; j < 8; j++) //8个段每个显示一下。共阳的有显示程序没有显示效果
    {
      app_tm1628.to_led_seg(bit(j), i + 1);
      delay_ms(300);
    }
  }

  //清屏，起始：LED1，共9个。也可单个操作
  //app_tm1628.to_led_clear(LED1,9);

  //ASCII码方式显示（单个LED操作，打点可选），码表上有ASCII的，才能操作
  //从物理LED序号1到9，显示1~9，每个LED上显示自己LED序号
  for (i = 0; i < 9; i++) //9个显示器
  {
    app_tm1628.to_led_char(' ',   NO_DOT, i + 1); //可用该语句将该显示器先清空（单个）
    if (i == 1) delay_ms(1000); //第一个清掉后，等一下显示。后面的一下子显示完
    app_tm1628.to_led_char('1' + i, DOT, i + 1); //1的ASCII码是33，可写app_tm1628.to_led_char(33+i,DOT, i+1)
  }
  delay_ms(1000);

  //数字方式显示
  app_tm1628.to_led_clear(LED1, 9);
  delay_ms(2000);
  //8位整数~32位长整数，支持负数，个位开始数，从1到第7位打点。注意，右定位
  //NO_DOT(0)：不打点；DOT1(1)：个位打点；DOT2(2)：十位打点，小数点后有1位...
  //显示"-87.654321"，注意"7."在一个位置上
  app_tm1628.to_led((int32_t)-87654321, DOT7, LED9, 9);
  delay_ms(2000);
  app_tm1628.to_led_clear(LED1, 9);
  //超量程，这3位显示"UUU     "
  app_tm1628.to_led((int32_t)-87654321, NO_DOT, LED3, 3);
  delay_ms(2000);
  app_tm1628.to_led_clear(LED1, 9);
  //浮点显示，四舍五入，小数点后一位，LED1LED2显示：" 1.5    "
  app_tm1628.to_led(1.456, DOT1, LED2, 2);
  delay_ms(2000);

}


void loop() {
  // put your main code here, to run repeatedly:
  //每50ms左右取一次键值，得键动作数据
  app_tm1628.get_key();
  delay_ms(48);

  //根据键动作，在LED3/LED4显示键号。本例只有4个按键，若有多个，可以每个测试
  unsigned char i = 0;
  for (i = 0; i < 4; i++)
  {
    if (app_tm1628.keys[i].key_buffer == KEY_DOWN)  app_tm1628.to_led((int16_t)i+1, NO_DOT, LED4, 2);
    else if (app_tm1628.keys[i].key_buffer == KEY_UP) app_tm1628.to_led_clear(LED4, 2);
  };

  //用按键控制来改变亮度（低亮度可低到关显示）
  //键号1-UP，2-DOWN，通过按这两键改变j。---这里不是键数组序号，已改为键号，是下标号+1
  //短按加减1(抬起为判断标志)，长按则逐渐加快加减的速度，最后是跳变（通过按下的时间来判断）。
  // unsigned int key_add_or_sub(unsigned char key_No, unsigned int variable,
  // unsigned int max_or_min, unsigned char add,
  //          unsigned char loop);  //loop暂时不起作用
  //j=0为关，1~71对应0~7共8档亮度。若亮度设为i，则数字为j=i*10+1,即10倍再平移1。1~10对应0档，2~20对应1档......
  static unsigned char j = 41; //初始亮度为中间值
  j = app_tm1628.key_add_or_sub(1, j, 71, 1, 0);  //keys[0]（下标号为0，即键1）
  j = app_tm1628.key_add_or_sub(2, j, 0, 0, 0);
  app_tm1628.to_led((int16_t)j, NO_DOT, LED6, 2);

  if (j == 0) app_tm1628.set_onoff(OFF);
  else
  {
    i = map(j, 1, 71, 0, 7) ; //查map原型，都是整数操作，没有四舍五入，小数舍去，所以1~10对应0
    /*
      long map(long x, long in_min, long in_max, long out_min, long out_max)
      {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
      }
    */
    app_tm1628.set_brightness(i);
  }
  //用串口查看一下map()前后的数的对应关系。
  Serial.print("j:");
  Serial.println(j);
  Serial.print("i:");
  Serial.println(i);

  //指示灯操作
  //to_led_seg(int8_t seg, uint8_t No)可以对物理LED显示器No进行按字节操作，这个前面演示过了。
  //如果要对14个指示灯进行操作，得对逻辑LED第8、9位按字节操作，前提是知道其对应的物理LED序号。
  //也可以通过专门的功能函数对指示灯进行操作，共三条指令：
  //    void set_lamp(enum_LAMP lamp_No);  设置单个或全部指示灯缓冲区
  //    void clr_lamp(enum_LAMP lamp_No);  清除单个或全部指示灯缓冲区
  //    void to_led_lamps(void);           写入到硬件，实现显示
  //LAMP1~LAMP7对应SEG2-GRID1~7，即缺省电路的第9位七段LED
  //LAMP8~LAMP14对应SEG1-GRID1~7，即缺省电路的第8位七段LED
  //LAMP_ALL所有指示灯
  app_tm1628.clr_lamp(LAMP_ALL);
  app_tm1628.set_lamp((enum_LAMP)(i + 1)); //亮度i=0~7通过LAMP1~LAMP8指示出来
  app_tm1628.to_led_lamps();
}

