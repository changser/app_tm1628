////////////////           TM1628的驱动库      V0.5            //////////////
////////////////     Copyright 2016-2017      by changser      //////////////
////////////////           changser@139.com                    //////////////

#include "driver_tm1628.h"

//TM1628构造函数。初始化相关变量,执行芯片的初始化（模式和亮度）。------------------------------
//输入：
//
TM1628:: TM1628(uint8_t stb , uint8_t dio , uint8_t clk )
  : stb_pin(stb), dio_pin(dio), clk_pin(clk), brightness(4)   //, mode(GRID7_SEG10), brightness(4)
{
  set_mode();         //取类声明时的函数默认值
  set_brightness(brightness);   //取类声明时的函数默认值
}

//功能：对TM1628器件所用端口进行初始化----------------------------------------------------------
//输入：无
//输出：管脚STB=H,CLK、DIO为输出
void TM1628::init_tm1628_PORT(void)
{
  pinMode(stb_pin, OUTPUT);
  digitalWrite(stb_pin, HIGH);
  pinMode(dio_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
}


//功能：向TM1628发送8位数据,从低位开始--------------------------------------------------------
//输入：uint8_t dat
//输出: 此模拟SPI口的CLK、DIO
void TM1628::send_8bit(uint8_t dat)
{
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    if (dat & 0x01)  digitalWrite(dio_pin, HIGH);
    else digitalWrite(dio_pin, LOW);
    digitalWrite(clk_pin, LOW);
    delay_us(1);
    digitalWrite(clk_pin, HIGH);
    dat = dat >> 1;
  };
}

//功能：向TM1628发送命令--------------------------------------------------------------------
//      注意：开始片选恢复未选中，再变低变选中。命令完毕仍为低，
//      因为读显示寄存器和写按键寄存器需紧跟相关命令。
//      所以，为了防干扰，不要长期处于选中。在与读写寄存器无关的命令发出之后，记得及时将STB变高。
//      不及时将STB变高也不影响功能，因为每个command()中一开始就置高STB。
//输入：uint8_t com
//输出：STB高变低、CLK、DIO
void TM1628::command(uint8_t com)
{
  digitalWrite(stb_pin, HIGH);
  delay_us(1);
  digitalWrite(stb_pin, LOW);
  send_8bit(com);
}

//功能：结束片选，即stb_pin变高。成员函数不用，直接用语句。提供给外部使用。------------------------
//输入：私有变量stb_pin
//输出：管脚stb_pin
void TM1628::end_cs(void)
{
  digitalWrite(stb_pin, HIGH);
}

//功能：设置器件tm1628工作模式，几段几位---------------------------------------------------------
//输入：enum enum_dismode{GRID4_SEG13 = 0, GRID5_SEG12 = 1, GRID6_SEG11 = 2, GRID7_SEG10 = 3} dismode
//输出：
void TM1628::set_mode(enum_dismode dismode) //设置器件工作模式，缺省7位10段
{
  // mode = dismode;
  init_tm1628_PORT();
  command(dismode);
  digitalWrite(stb_pin, HIGH);
}

//功能：亮或灭TM1628，不影响原先亮度设置-------------------------------------------------------
//输入：uint8_t onoff
//输出：显示亮或灭
void TM1628::set_onoff(uint8_t onoff)  //亮（非0）或灭（0），硬件操作
{
  uint8_t dis_ctr_com = 0b10000000; //第7位为1，第6位为0，显示控制命令
  if (onoff)  bitSet(dis_ctr_com, 3); //需点亮，则置位第3位
  dis_ctr_com  += brightness;         //亮或灭不影响亮度。
  init_tm1628_PORT();
  command(dis_ctr_com);
  digitalWrite(stb_pin, HIGH);
}

//功能：改变TM1628亮度，同时点亮--------------------------------------------------------------
//输入：uint8_t level,0~7,其他为7
//输出：TM1628:brightness
//      显示亮度改变
void TM1628::set_brightness(uint8_t level)  //level:0~7共8级亮度，硬件操作，设置亮度时会同时点亮
{
  if (level > 7)  level = 7;
  brightness = level;
  uint8_t dis_ctr_com = 0b10001000; //第7位为1，第6位为0，显示控制命令；第3位为1，点亮
  dis_ctr_com  += brightness;         //同时点亮
  init_tm1628_PORT();
  command(dis_ctr_com);
  digitalWrite(stb_pin, HIGH);
}

//功能：设置待操作的显示寄存器的地址，设置是否自动加。硬件操作--------------------------------------
//      同时自动设为寄存器写模式（显示寄存器写），后面直接跟写操作。按键寄存器不能设置地址。
//输入：uint8_t addr，0~13，超过了器件会忽略
//      uint8_t inc，非0则自动加
//输出：
void TM1628::set_address(uint8_t addr , uint8_t inc )
{
  uint8_t data_com = 0b01000000; //第二位为0，自动增加；第0和1位为00，寄存器写
  if (!inc) bitSet(data_com, 2);     //第二位为1则为固定地址
  init_tm1628_PORT();
  command(data_com);
  uint8_t addr_com = 0b11000000;
  addr_com += addr;
  command(addr_com);
  //digitalWrite(stb_pin, HIGH);不能有，因为需跟写入的数据。地址自动增1方式下（本程序缺省值）单个或多个数据后再变高。
}

//功能：写显示寄存器，硬件操作，不包含set_address()内容，实质为send_8bit(data)---------------------------------
//      第一个需跟在set_address()之后。不改变为读按键寄存器的话，则可再跟第二个或更多。
//      注意：若是自动加1的方式（本程序都是这种方式），可以多个数据连接写，中间STB不用变高。
//输入：uint8_t data，要显示的段码SEG1~SEG14(缺省是GRID7_SEG10模式，则只有SEG1~SEG10)
//输出：显示内容
void TM1628::write_disR(uint8_t data)
{
  send_8bit(data);
  //digitalWrite(stb_pin, HIGH);不能有，因为需跟写入的数据。地址自动增1方式下（本程序缺省值）单个或多个数据后再变高。
}

//功能：读取按键寄存器并存入key_R[]数组，一一对应。从低字节开始，从低位开始-----------------------------------
//输入：TM1628按键寄存器
//输出：key_R[]
void TM1628::read_keyR()
{
  uint8_t i, j;
  init_tm1628_PORT();
  command(0x42);  //读键盘命令
  pinMode(dio_pin, INPUT_PULLUP);
  for (j = 0; j < 5; j++) //连续读取5个字节
  {
    for (i = 0; i < 8; i++)
    {
      key_R[j] = key_R[j] >> 1;
      digitalWrite(clk_pin, LOW);
      delay_us(1);
      digitalWrite(clk_pin, HIGH);
      delay_us(1);
      if (digitalRead(dio_pin))
      {
        key_R[j] = key_R[j] | 0X80;
      }
    }
    //问题：水检仪项目发现：去除键盘显示面板后，电机自动转动起来。原因是所有按键值读为高（口线上拉了或不确定），判断CCW键有效。
    //解决办法1、硬件方法：单片机端加入下拉，需考虑去除内部上拉和与键盘端匹配，应是键盘端上拉2倍左右。
    //         或   若内部上拉仍有效，则两个外部上拉，则下拉值跟上拉值一样.
    //办法2、软件方法：若读得一字节的都为高，则肯定是面板未接入。因为TM1628的B6\B7应为0。此时读数应无效，将所有位设置为0。
    if (key_R[j] == 0xff) key_R[j] = 0;
  }
  digitalWrite(stb_pin, HIGH);
}

