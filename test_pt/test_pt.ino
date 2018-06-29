////////////////      APP_TM1628(含TM1628类)类库应用程序例程                          ////////////
////////////////      test_pt   V180628                                               ////////////
////////////////      by changser    20180628                                         ////////////
////////////////      changser@139.com                                                ////////////

//说明：
//APP_TM1628在协程changser_pt_for_arduino（V20170611）平台上的应用例子。
//该协程由本人收集整理，其中pt-signal.h由我建立，原始PT由Adam Dunkels建立，pt-timer.h作者应是“逍遥猪葛亮”

//更改记录：
//V180628       建立，IDE基于1.8.1，APP_TM1628类库版本V0.6，changser_pt_for_arduino版本V20170611

//////////////////////////////////////////////////////////////////////////////////////////////////
//用户头文件
#define PT_USE_TIMER
#define PT_USE_SIGNAL
#define LC_INCLUDE "lc-addrlabels.h"

#include "pt.h"

#include "dis_key.h"

//变量常量定义：

//设置物理LED显示器映射表。
static const uint8_t dis_table1[9] = {6, 5, 4 , 3, 2, 1, 7, 8, 9 };

//设置物理按键映射表。
static const uint8_t key_table1[] = {1, 4, 5, 8};

//实例化
APP_TM1628 app_tm1628 = APP_TM1628(11, 12, 13); //STB=11,DIO=12,CLK=13

static struct pt thread1, thread2, thread3;
static int thread1_entry(struct pt *pt);
static int thread2_entry(struct pt *pt);
static int thread3_entry(struct pt *pt);

/////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  //Initialize the Threads
  PT_INIT(&thread1);
  PT_INIT(&thread2);
  PT_INIT(&thread3);


 // pinMode(13, OUTPUT);  //第13脚被TM1628用了，这里不用这个灯，用TM1628驱动的灯
 // digitalWrite(13, LOW);

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
}


void loop() {
  //将任务与pt结构体变量关联，且不断从让出点再进入。
  thread1_entry(&thread1);
  thread2_entry(&thread2);
  thread3_entry(&thread3);
}

//任务1：每50ms取键盘状态更新keys[]，其它任务查询keys[]就好。
//也可以在此任务中判断按键动作再发信号给相应的任务（其它任务等信号）。
static int thread1_entry(struct pt *pt)
{
  PT_BEGIN(pt);
  while (1)
  {
    app_tm1628.get_key();
    PT_WAIT_MS(pt, 50);
  }
  PT_END(pt);
}


//任务2：不断查询按键1的按下动作。查到发信号给任务3
static int thread2_entry(struct pt *pt)
{
  PT_BEGIN(pt);
  while (1)
  {
    if (app_tm1628.keys[0].key_buffer == KEY_DOWN) PT_SEND_SIGNAL(&thread3);
    PT_YIELD(pt); //Check the other events.关键就在于这个出让，否则还是会转到本任务开始执行
  }
  PT_END(pt);
}


uint8_t i;  //下面任务中，基于i的for循环之中，有出让控制权，i的值在下次进入时仍有用，所以不能用局部变量。
//任务3：等发信号过来，等到了就闪两下。等时不占用单片机时间；灯亮或灭0.5s期间也没有占用单片机时间，出让控制权了。
static int thread3_entry(struct pt *pt)
{
  PT_BEGIN(pt);
  while (1)
  {
    PT_WAIT_SIGNAL(pt);
    for (i = 0; i < 2; i++)  //闪两下
    {
      //digitalWrite(13, HIGH);
      app_tm1628.to_led_seg(bit(0), 7); //LED1的a段亮
      app_tm1628.set_lamp(LAMP1); //LAMP1置位
      app_tm1628.to_led_lamps();  //写入芯片，点亮
      PT_WAIT_MS(pt, 500); //或PT_TIMER_DELAY(pt, 200);
      
      //digitalWrite(13, LOW);   
      app_tm1628.to_led_seg(0, 7); //LED1的a段灭
      app_tm1628.clr_lamp(LAMP1); //LAMP1清0
      app_tm1628.to_led_lamps();  //写入器件，灭
      PT_WAIT_MS(pt, 500); //PT_TIMER_DELAY(pt, 200);
    }
  }
  PT_END(pt);
}

