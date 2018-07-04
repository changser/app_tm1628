////////////////      APP_TM1628(含TM1628类)类库应用程序例程                          ////////////
////////////////      test_pt2   V180704                                               ////////////
////////////////      by changser    20180704                                         ////////////
////////////////      changser@139.com                                                ////////////

//说明：
//APP_TM1628在协程changser_pt_for_arduino（V20170611）平台上的应用例子3。
//该协程由本人收集整理，其中pt-signal.h由我建立，原始PT由Adam Dunkels建立，pt-timer.h作者应是“逍遥猪葛亮”
//本例的目的是为了对类库新增功能函数进行验证：key_func_done(uint8_t key_no)
//功能是：上电时所有指示都闪一下；上电进行任务2，对应指示灯亮。判断键1是否按下，按下键1后指示灯灭并转到任务3；
//          任务3时对应指示灯亮，判断键1是否按下，按下键1后指示灯灭并转到任务2；
//          从指示灯的角度来看，就是用两个指示灯代表两个状态，键1按一下就切换到另一个状态。
//          本例子与test_pt1的区别就是：本例用的是一个键，test_pt1用的是两个键。
//          用一个键的话，当按下键时，其KEY_DOWN或KEY_UP动作，在任务2中判断之后，进入任务3，有可能被认为又被按下了。
//          所以，某键的KEY_DOWN或KEY_UP功能被执行之后，可以调用key_func_done(uint8_t key_no)功能，以消除这个键动作标志。

//更改记录：
//V180704       建立，IDE基于1.8.1，APP_TM1628类库版本V0.7，changser_pt_for_arduino版本V20170611

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
  PT_INIT(&thread1); //只对结构thread1中的lc元素初台化，对t和signal并不处理
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

  //PT_SEND_SIGNAL(&thread2);  //不能放在这里，发送信号的本质是将tread2的signal元素置1.但此时thread2_entry(&thread2);还没有运行过。
  //thread2.entry(&thread2)的中的PT_WAIT_SIGNAL(pt);也没有运行中。
  //PT_WAIT_SIGNAL(pt);运行之中，在放出控制权之前有清signal为0的语句。此后才等signal信号。
  //所以，等信号之后，再发信号才会有用。否则会被清掉。
  //等信号过程中，再重复运行thread2.entry(&thread2);就会跳到等待处，不会再运行前面的清0语句。这涉及协程本质（switch跳转）。
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


//任务2：上电就运行的任务。只运行一次。不断查询按键1的按下动作。查到发信号给任务3，并等一下运行（别的任务发信号过来）
static int thread2_entry(struct pt *pt)
{
  PT_BEGIN(pt);
  //PT_WAIT_SIGNAL(pt);   //若要上电就运行一次，这条放这里等setup()里发的信号的话，行不通，放在末尾。见setup里发信号语句取消的说明。
 while (1)             //这个循环体可以不要。因为协程切换的本质在于任务被不断调用（比如loop()中），PT_BEGIN(pt);是switch，出让点是case分支。
                         //有没有这个while(1)，下次都会在出让点继续，因为每次loop()被调用又会重复switch case 来跳转。
                         //但是最好还是要，因为如果while(1)前面如果还有语句，那么执行次数是不一样的；
                         //另外，要了的话，看起来像是一个任务，更好理解，以后移植到OS也更方便。
  {
    app_tm1628.to_led_seg(bit(0), 7); //LED7的a段亮
    app_tm1628.set_lamp(LAMP1); //LAMP1置位
    app_tm1628.to_led_lamps();  //写入芯片，点亮
    while (1)  //内循环用于不断处理输入输出等情况。一定要有出让控制权。
    {
      if (app_tm1628.keys[0].key_buffer == KEY_DOWN) //键1对应keys[0]
      {
        app_tm1628.key_func_done(1); //清键1的KEY_DOWN状态为KEY_KEEP。可以取消看有没有任务2和3之间跳几下。
        app_tm1628.to_led_seg(0, 7); //LED7的全段灭
        app_tm1628.clr_lamp(LAMP1); //LAMP1清0
        app_tm1628.to_led_lamps();  //写入器件，灭
        PT_SEND_SIGNAL(&thread3);
        break;
      }
      PT_YIELD(pt); //Check the other events.关键就在于这个出让，否则还是会不断循环，直到按键有效
    }
    PT_WAIT_SIGNAL(pt);
  }
  PT_END(pt);
}


//任务3：等发信号过来，等到了就闪两下。等时不占用单片机时间；灯亮或灭0.5s期间也没有占用单片机时间，出让控制权了。
static int thread3_entry(struct pt *pt)
{
  PT_BEGIN(pt);
  while (1)    
  {
    PT_WAIT_SIGNAL(pt);
    app_tm1628.to_led_seg(bit(1), 7); //LED7的B段亮
    app_tm1628.set_lamp(LAMP2); //LAMP2置位
    app_tm1628.to_led_lamps();  //写入芯片，点亮
    while (1)
    {
      if (app_tm1628.keys[0].key_buffer == KEY_DOWN)  //如果任务2、3都用一个键的按下动作来判断的话，会出现问题。
                                                      //因为可能会是同一个按下动作，但切换到下一个任务之后，
                                                      //该键动作还有效，这不是期望的。
                                                      //解决办法，一个用键按下，另一个用键松开（不一定有用，要看先后顺序）；
                                                      //或者应增加一个消除键动作的过程。
      {
        app_tm1628.key_func_done(1);//清键1的KEY_DOWN状态为KEY_KEEP。可以取消看有没有任务2和3之间跳几下。
        app_tm1628.to_led_seg(0, 7); //LED7的a段灭
        app_tm1628.clr_lamp(LAMP2); //LAMP1清0
        app_tm1628.to_led_lamps();  //写入器件，灭
        PT_SEND_SIGNAL(&thread2);
        break;
      }
      PT_YIELD(pt); //Check the other events.关键就在于这个出让
    }
  }
  PT_END(pt);
}

