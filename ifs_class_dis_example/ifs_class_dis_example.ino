////////////////      APP_TM1628(含TM1628类)类库应用程序例程                          ////////////      
////////////////      ifs_class_dis_example   V170226                                 ////////////
////////////////      by changser    20170319                                         ////////////
////////////////      changser@139.com                                                ////////////

//说明：
//在界面或菜单跳转框架程序例程ifs_class_serial_example基础上改变而来，所以必需有IFS类库支持
//在上述示例的基础上，加入了键盘显示部分的互动。可以说是两个库的联合示例。
//原功能为：
//界面跳转演示，共三个界面，通过串口文本来显示各界面进入、即时运行和退出的时机
//控制也是通过键盘输入串口发送来实现。上翻w,下翻s
//enter1............................................
//running1..
//bye1..
//enter2..........................................
//...
//现加入：
//1、上电闪灯一下
//2、加上keys[0]当作上翻键，keys[1]当作下翻键。原有电脑键盘的w\s上下翻仍有效。
//3、加入翻过之后，LED的显示。

//更改记录：
//V170319   本文件，基于IDE1.8.1，硬件UNO和M0（未测试）。
//          在MO硬件平台上，易在to_led()处出现 call of XXX overloaded is ambiguous
//          原因是重入的问题。在示例中调用to_led()时，对要显示参数进行明确的类型强制转换。
//V170226   基于IDE1.8.1，硬件UNO
//          1、完善注释，变量都转换成类似uint8_t类型
//          2、更改IFS类实例所需的变量表定义，主要是去掉const属性，因为IDE1.8.1警告说是无效转换
//V161228   建立，基于IDE 1.6.11

//用户头文件
#include <ifs_class.h>
#include "dis_key.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//内部函数声明：
//功能：根据串口命令解析得到界面切换命令
//输入：串口缓冲区
//输出：static enum switchCMDenum类型
static enum switchCMDenum getIfsCMD();

//功能：界面1的进入、即时运行、退出函数
static void enter1();
static void run1();
static void exit1();

//功能：界面2的进入、即时运行、退出函数
static void enter2();
static void run2();
static void exit2();

//功能：界面3的进入、即时运行、退出函数
static void enter3();
static void run3();
static void exit3();

//////////////////////////////////////////////////////////////////////////////////////////////////
//变常量定义：
//界面结构体数组
#define IFSNUM  3
static  interfaceStr interfaceTable[IFSNUM]
= {
  //{0,{0,0,0,0,0,0},NULL,NULL,NULL},标准格式，0或空为初始值
  {1, {3, 2, 0, 0, 0, 0}, enter1, run1, exit1}, //缺省为开机的入口界面1。必须是数组第一个（下标为0）
  {2, {1, 3, 0, 0, 0, 0}, enter2, run2, exit2},
  {3, {2, 1, 0, 0, 0, 0}, enter3, run3, exit3}
};

//类IFS实例为对象ifs
IFS ifs = IFS(&interfaceTable[0], IFSNUM);

//设置物理LED显示器（下标+1）与逻辑LED显示器序号（元素）的映射表。即排位顺序。一定要是9个？实际没有的怎么处理？
static const uint8_t dis_table1[9] = {6, 5, 4 , 3, 2, 1, 7, 8, 9 };

//设置物理按键序号（下标+1）与逻辑按键序号（元素）的映射表（1~20）。即排位顺序。最大20个元素。
static  const uint8_t key_table1[] = {1, 4, 5, 8};

APP_TM1628 app_tm1628 = APP_TM1628(11, 12, 13);

/////////////////////////////////////////////////////////////////////////////////////////////
//arduino开机初始化，系统默认
void setup() {
  // put your setup code here, to run once:
  app_tm1628.set_dis_table(dis_table1);  //dis_table1 <=> &dis_table1[0]
  app_tm1628.set_key_table(key_table1, sizeof(key_table1));
  Serial.begin(9600);
  Serial.println("hello,welcome");
  app_tm1628.to_led_flash();  //所有显示闪一下，包括指示灯
}

//arduino循环，系统默认。
void loop() {
  // put your main code here, to run repeatedly:
  ifs.switchCMD = getIfsCMD();
  ifs.doSwitchCMD();  //执行窗口解析函数
  delay(48);
  app_tm1628.get_key();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//函数实现：
//功能：根据串口命令解析得到界面切换命令
//输入：串口缓冲区
//输出：static enum switchCMDenum类型
static enum switchCMDenum getIfsCMD()
{
  enum switchCMDenum temp = SNONE;
  if (Serial.available() > 0)
  {
    switch (Serial.read())
    {
      case 'w':
        temp = SUP;
        break;
      case 's':
        temp = SDOWN;
        break;
      default:
        temp = SNONE;
        break;
    }
  };
  if (temp == SNONE)
  {
    if (app_tm1628.keys[0].key_buffer == KEY_DOWN) temp = SUP;
    else  if (app_tm1628.keys[1].key_buffer == KEY_DOWN) temp = SDOWN;
  };
  return temp;
}

static void enter1()
{
  //界面初始化，如重生成新屏、参数初始化
  Serial.println("hello");
  Serial.println("enter1............................................");
  app_tm1628.to_led_seg(0b10000000, LED5);
  app_tm1628.to_led_char('-', NO_DOT, LED6);
  app_tm1628.to_led(1.456, DOT2, LED4, 4);
}

static void run1()
{
  //显示涮新或其他实时动作
  static unsigned int  i = 0;
  i++;
  if (i >= 20)
  {
    i = 0;
    Serial.println("running1..");
  }
}

static void exit1()
{
  //退出界面时参数销毁或保存等
  Serial.println("bye1");
}

static void enter2()
{
  Serial.println("enter2...................................................");
  app_tm1628.to_led((int16_t)5432, NO_DOT, LED4, 4 );
}

static void run2()
{
  static unsigned int i = 0;
  i++;
  if (i >= 20)
  {
    i = 0;
    Serial.println("running2..");
  }
}

static void exit2()
{
  Serial.println("bye2");
}

static void enter3()
{
  Serial.println("enter3.....................................................");
  app_tm1628.to_led((int16_t)76543, NO_DOT, LED5, 5);
}

static void run3()
{
  static unsigned int i = 0;
  i++;
  if (i >= 20)
  {
    i = 0;
    Serial.println("running3..");
  }
}

static void exit3()
{
  Serial.println("bye3");
}

