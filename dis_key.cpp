////////////////           TM1628驱动库的应用库      V0.6       //////////////
////////////////     Copyright 2016-2018      by changser       //////////////
////////////////           changser@139.com                     ///////////////////////////
 
#include  "dis_key.h"

//定义显示段码表，表中每个元素有两个字节对应，一个ASCII，一个八段码。可以由ASCII查找八段码。
//格式，dp,gfedcba->seg8,seg7..seg1,共阴，置1时该段显示。
//TM1628中seg1是显示缓冲字节的低位。
static const struct {
  int8_t ascii;
  uint8_t seg;
} char_array[]
= {
  0x0, 0b00000000,   //NUL,空
  '0', 0b00111111,   //0,无小数点（下同）
  '1', 0b00000110,   //1
  '2', 0b01011011,   //2
  '3', 0b01001111,   //3
  '4', 0b01100110,   //4
  '5', 0b01101101,   //5
  '6', 0b01111101,   //6
  '7', 0b00000111,   //7
  '8', 0b01111111,   //8
  '9', 0b01101111,   //9
  'A', 0b01110111,   //A
  'a', 0b01110111,
  'B', 0b01111100,
  'b', 0b01111100,
  'C', 0b00111001,
  'c', 0b00111001,
  'D', 0b01011110,
  'd', 0b01011110,
  'E', 0b01111001,
  'e', 0b01111001,
  'F', 0b01110001,
  'f', 0b01110001,
  'U', 0b00111110,
  'u', 0b00111110,
  'O', 0b01011100,
  'o', 0b01011100,
  'R', 0b00110001,
  'r', 0b00110001,
  'P', 0b01110011,
  'p', 0b01110011,
  'S', 0b01101101,
  's', 0b01101101,
  'K', 0b01110101,
  'k', 0b01110101,
  'I', 0b00000101,
  'i', 0b00000101,
  'T', 0b01111000,
  't', 0b01111000,
  'L', 0b00111000,
  'l', 0b00111000,
  '-', 0b01000000
};

//物理键盘与逻辑键盘映射表。一维数组。每个物理键盘（序号为元素下标+1）对应的逻辑键盘序号1~20
//一个数组下标代表一个物理键盘序号，从0到KEY_NUM-1，对应从1到KEY_NUM个物理按键
//每个元素值代表一个逻辑键盘的编号（以标准电路为准）
//若实际按键排列与标准电路一致，则程序算法也与其一致，不需要此表。
//若实际按键排列与标准电路不一致，则在用户程序中设置此表（数组名可变），
//用户程序初始化时由  实例名.set_dis_table(&dis_table[0]); 来关联。若没有关联，则程序算法与逻辑表相一致。
//使用这种方法的好处是：随意定义键盘序号；只取有用的键的序号，避免keys[]的内存空间浪费。
//static const uint8_t key_table[KEY_NUM] = {1, 4, 5, 8};

//显示映射表。一维数组。每个物理LED（序号为元素下标+1）对应的逻辑LED序号1~9
//逻辑LED对应TM1628内部显示寄存器依次为0,2,4,6,8,10,12(共阴)及1\3\5\7\9\11\13的SEG1、SEG2(共阳)。
//每个数组下标+1表示实际物理LED的编号，LED排列缺省从左往右，从1开始。
//每个元素值表示逻辑LED序号（从1~9），以标准电路为准。
//因为硬件设计的关系，显示寄存器偶数地址才对应一个完整的LED，有0,2,4,6,8,10,12,共7个共阴LED
//另有两个共阳LED，分别占用每个奇地址的SEG1\SEG2，即位0和位1，占用GRID1~7
//本映射表表明的是缺省的逻辑LED显示器排列。若与此一致，则不必要。
//若实理LED显示器排列与此不同，需在用户程序中设置此表（数组名可变），个数一定要是9个?
//用户程序初始化时由  实例名.set_dis_table(&dis_table[0]); 来关联。若没有关联，则程序算法与逻辑LED表相一致。
//static const uint8_t dis_table[9] = {1, 2, 3 , 4, 5, 6, 7, 8, 9 };

APP_TM1628::APP_TM1628(uint8_t stb, uint8_t dio , uint8_t clk ): TM1628(stb, dio, clk)
{
  //keys = new struct_key [20];
  //keys = keyss;
}

//若更改物理七段LED顺序，设置新的映射表，缺省的逻辑LED显示器顺序（没有用表）无效。
void APP_TM1628::set_dis_table(uint8_t const *p)
{
  dis_table_P = p;
}

//若更改物理按键的键序，设置新的映射表，缺省的逻辑按键顺序（没有用表）无效。
void APP_TM1628::set_key_table(uint8_t const *p, uint8_t keynum)
{
  key_table_P = p;
  //delete[] keys;
  key_num = keynum;
  //keys = new struct_key[4];
  //pkeys = keyss;

}

//功能：字符ASCII转SEG码
//输入：char val，某ASCII码
//输出：uint8_t，根据char_array[]码表，由ASCII查对应的八段码
uint8_t APP_TM1628::char_to_seg(char val)
{
  uint8_t i;
  for (i = 0; i < sizeof(char_array) / 2; i++)
  {
    if (val == char_array[i].ascii) break;
  }
  if (i < sizeof(char_array) / 2) return char_array[i].seg;
  else return 0;  //段码为0，则无显示
}

//功能：查表，七段LED物理位置序号转逻辑LED位置序号
//物理位置是指从左到右的LED编号，从1~9
//逻辑位置是指缺省电路的LED编号，从1~9。主要特征是与TM1628的显示寄存器有固定的对应关系。
//逻辑序号i=1~7对应寄存器(2*(i-1)),逻辑序号8对应(2*(i-1)+1)（i从0~7）的SEG1,逻辑序号9对应SEG2。
//输入：uint8_t ledNo:七段LED物理充号，从左到右，从1到9
//输出：uint8_t，根据dis_table[]表，由物理LED序号，查对应的逻辑LED序号
uint8_t APP_TM1628::ledNo_to_ledLogNo(uint8_t ledNo)
{
  if (dis_table_P == NULL) return  ledNo;
  else return dis_table_P[ledNo - 1];
}

//置位某指示灯，或所有指示灯，只是类变量更改，没涮新硬件
//LAMP1~7对应SEG2-GRID1~7，即缺省电路的第9位七段LED
//LAMP8~14对应SEG1-GRID1~7，即缺省电路的第8位七段LED
//LAMP_ALL所有指示灯
void APP_TM1628::set_lamp(enum_LAMP lamp_No)
{
  if (lamp_No == LAMP_ALL) twoCA = 0xffff;
  else if (lamp_No < 8) bitSet(twoCA, lamp_No - 1);
  else bitSet(twoCA, lamp_No);
}

//清除某指示灯，或所有指示灯，只是类变量更改，没涮新硬件
void APP_TM1628::clr_lamp(enum_LAMP lamp_No)
{
  if (lamp_No == LAMP_ALL) twoCA = 0;
  else if (lamp_No < 8) bitClear(twoCA, lamp_No - 1);
  else bitClear(twoCA, lamp_No);
}

//功能：涮新TM1628奇地址寄存器，实现显示。两个共阳七段LED或14个灯一起涮新
//输入：APP_TM1628::twoCA
//输出：TM1628奇显示寄存器
void APP_TM1628::to_led_lamps(void)
{
  uint8_t i, temp, tempH, tempL;
  tempH = highByte(twoCA);
  tempL = lowByte(twoCA);
  for (i = 0; i < 7; i++)
  {
    temp = 0;
    //逻辑显示器8，SEG1-GRID1~7，LED8~14，存于twoCA高位
    bitWrite(temp, 0, bitRead(tempH, i));
    //逻辑显示器9，SEG2-GRID1~7，LED1~7，存于twoCA低位
    bitWrite(temp, 1, bitRead(tempL, i));
    set_address(2 * i + 1);
    write_disR(temp);
  }
  end_cs();
}

//功能：将所有显示器设置为显示8.即字段全亮，包括指示灯.
//输入：无
//输出：APP_TM1628::twoCA，TM1628显示寄存器
void  APP_TM1628:: to_led_all8()
{
  uint8_t i;
  set_address(0);
  for (i = 0; i < 14; i++)
  {
    write_disR(0b11111111); //不用查表了，所有字段都为1，即点亮，包括小数点
  };
  //同时涮新奇数寄存器的对应的属性值
  set_lamp(LAMP_ALL);
  end_cs();
}

//功能：将某几个显示器设置为显示空，包括指示灯
//      暂时对超过LED_NUM的值不做处理
//输入：
//uint8_t start_led:从哪个物理七段LED开始,1~9
//unsigned led_long：不显示的七段LED个数
//输出：可能有APP_TM1628::twoCA，TM1628显示寄存器
void APP_TM1628::to_led_clear(uint8_t start_led, uint8_t led_long)
{
  uint8_t i;
  for (i = 0; i < led_long; i++)
  {
    to_led_char(' ', NO_DOT, start_led + i);
  };
  end_cs();
}

//功能：将SEG码直接在TM1628驱动下显示出。（高位->）dp,gfedcba(<-0位)
//输入：
//  int8_t seg  ：要显示的SEG码。
//  uint8_t No  ：物理显示器序号1~9
//输出：可能有APP_TM1628::twoCA，TM1628显示寄存器
void APP_TM1628::to_led_seg(int8_t seg, uint8_t No)
{
  uint8_t temp1=0;
  temp1 = ledNo_to_ledLogNo(No);
  if (temp1 < 8)  //逻辑显示器1~7
  {
    set_address(2 * (temp1 - 1));
    write_disR(seg);
  }
  else //逻辑显示器8、9
  {
    uint16_t temp=0;
    if (temp1 == 8) //共阳LED，TM1628的SEG1对应奇地址显示寄存器，灯LAMP8~14
    {
      temp = twoCA & 0XFF;  //LAMP8~14存于高位字节。高位清0
      temp += seg << 8;
    } else if (temp1 == 9)//共阳LED，TM1628的SEG2对应奇地址显示寄存器，灯LAMP1~7
    {
      temp = twoCA & 0XFF00;  //LAMP1~7存于低位字节。低位清0
      temp += seg ;
    }
    twoCA = temp;
    to_led_lamps(); //  涮新硬件
  }
  end_cs();
}

//功能：将ASCII对应的单个字符在TM1628驱动下显示出来。可选是否加小数点。
//      注意，若是共阳的两个LED，则小数点实际是无效的，显示不出来。
//输入：
//char val：ASCII值。可直接用65或'A'。
//bool dot ：是否加小数点，不加为false。加小数点为true.
//uint8_t No：决定物理LED的一个位置，为LED1(1)...LED9(9)
//输出：
//可能有APP_TM1628::twoCA，TM1628显示寄存器
void APP_TM1628::to_led_char(int8_t ch , uint8_t dot, uint8_t No  )
{
  char temp;
  temp = char_to_seg(ch);
  if (dot) temp += 0b10000000;
  to_led_seg(temp, No);
}

//功能：32位整数数字作为10进制字符串显示。以个位为基准，在选定位置及左边显示。
//      物理显示器排序为从左到右是LED1~LED9。加入小数点处理。
//输入：
//signed long val:    要转换的数。long型为10位，一般都够了。
//uint8_t dot:    决定小数点在val上的位置，从个位数开始，取值为DOT1，则个位有小数点。
//                NO_DOT(0)、DOT1(1)、DOT2(2)、DOT3(3)...DOT9(9)，NO_DOT(即0)表示无小数点。
//uint8_t No:     决定个位在物理LED的位置，为LED1(1)...LED9(9)。高位显示在左边，LED序号值比个位的小。
//uint8_t len:    本次显示用到的LED的个数
//输出：
//可能有APP_TM1628::twoCA，TM1628显示寄存器
//uint8_t APP_TM1628::to_led(int32_t val, uint8_t dot = NO_DOT, uint8_t No = 4, uint8_t len = 4)
uint8_t APP_TM1628::to_led(int32_t val, uint8_t dot , uint8_t No , uint8_t len )
{
  uint8_t i, err = 0,  led_pos, led_len, str_len;//, over = 0;
  led_pos = No;   //个位物理LED的位置，
  led_len = len;  //
  String temp = "";
  temp += val; //若输入的数超过int类型呢？
  str_len = temp.length();

  //错误判断
  if ((led_len > 9) || (led_len == 0)) //占用的LED个数不对，.一共只支持1~9个
  {
    err = 1;
    led_len = 1;
  }
  else  if ((led_pos > 9) || (led_pos == 0)) //物理LED的个位序号不对，就1~9
  {
    err = 1;
    led_pos = 1;
  }
  else  if (led_len > led_pos) //设置时就显示不下。位置为3，长度为5，占用-1、0、1、2、3，没有-1和0，所以不对。
  {
    err = 1;
    led_len = led_pos;
  }
  else if (dot > led_len) //dot是从个位开始的数字的dot位打点，从1开始算位。dot位大于显示长度了，肯定不对
  {
    err = 1;
  }

  //错误处理，显示E
  if (err)
  {
    to_led_char('e',  led_pos, false );
  }
  else  //非错误
  {
    if (str_len > led_len)  //字符超长(超过设定值，可认为超量程)
    {
      //则当前位置及有效显示长度显示UU...
      for (i = 0; i < led_len; i++)
      {
        to_led_char('U', NO_DOT, (led_pos - i));
      };
    }
    else  //正常处理
    {

      ////处理如0.9的前面填0的情况。根据小数点填充0
      if (str_len < dot)
      {
        for (i = 0; i < (dot - str_len); i++)
        {
          temp = '0' + temp;
        }
        str_len = dot;
      }
      //字符长度不够led_len的，前面填充' ',免得原有字符不能去除。
      if (str_len < led_len)
      {
        for (i = 0; i < (led_len - str_len); i++)
        {
          temp = ' ' + temp;
        }
        str_len = led_len;
      }
      //显示led_len长度的字符串，加点处理不是改变字符，而是地接操作SEG码
      uint8_t have_dot ;
      for (i = 0; i < led_len; i++) //从个位开始显示
      {
        if (i == (dot - 1)) have_dot = DOT;
        else have_dot = NO_DOT;
        to_led_char(temp.charAt(str_len - i - 1),  have_dot, (led_pos - i));
      }
    }
  }//end of: "else" of "if (err)"
  end_cs();
  return err;
}

//功能：16位整数数字作为10进制字符串显示。以个位为基准。
//输入：
//int16_t number:    要转换的数。long型为10位，一般都够了。
//uint8_t dot:    决定小数点在val上的位置，从个数开始，取值为DOT1，则个位有小数点。
//               NO_DOT(0)、DOT1(1)、DOT2(2)、DOT3(3)...DOT9(9)，NO_DOT(即0)表示无小数点。
//uint8_t No:     决定个位在物理LED的位置，为LED1(1)...LED9(9)
//uint8_t len:    本次显示用到的LED的个数
//输出：
///可能有APP_TM1628::twoCA，TM1628显示寄存器
//uint8_t  APP_TM1628::to_led(int16_t number, uint8_t dot = NO_DOT, uint8_t No = 4 , uint8_t len = 4 )
uint8_t  APP_TM1628::to_led(int16_t number, uint8_t dot , uint8_t No  , uint8_t len  )
{
  return to_led((int32_t)number, dot, No, len);
}

//功能：将浮点数当作字符串在TM1628驱动下显示出来。以小数位为基准，此时DOT值为有几个小数位。缺省小数点后两位即DOT2。
//输入：
//signed float val:    要转换的数。ARDUINO AVR中，双精度浮点double与单精度float相同，都占4字节
//                     long型为10位，一般都够了。
//uint8_t dot:    决定小数点后数字个数，取值为DOT1，则有1个小数位。
//               NO_DOT(0)、DOT1(1)、DOT2(2)、DOT3(3)...DOT9(9)，NO_DOT(即0)表示无小数位。
//uint8_t No:     决定个位在物理LED的位置，为LED1(1)...LED9(9)
//uint8_t len:    本次显示用到的LED的个数
//输出：
///可能有APP_TM1628::twoCA，TM1628显示寄存器
//uint8_t  APP_TM1628::to_led(double val, uint8_t dot = DOT1, uint8_t No = 4 , uint8_t len = 4 )
uint8_t  APP_TM1628::to_led(double val, uint8_t dot , uint8_t No, uint8_t len )
{
  double temp;
  temp = val * pow(10, dot);//10的dot次方
  return to_led((int32_t)(temp + 0.5), dot + 1, No, len); //注意：四舍五入到了最右边的显示位
}

//功能：显示界面测试之-，在主循环之前，所有显示闪一下。
//输入：无
//输出：APP_TM1628::twoCA，TM1628显示寄存器
void APP_TM1628::to_led_flash()
{
  to_led_all8();
  delay_ms(1000);
  to_led_clear(0, 9);
  delay_ms(1000);
}

//功能：取键子函数，结果存放于键盘缓冲区中keys[20]中，对键的四个动作都有描述
//        如果存在按键映射表，只处理表中的几个键，keys[i]中序号i=物理按键序号-1
//采用的方法是：基于原先的键动作状态：按下、按下保持及时间、抬起、未按下保持及时间
//              根据新的key_state[]，得到新状态。时间是计数。运行一次计算一次。
//输入：  调用read_keyR()函数，再得key_state[key_num]
//        其中，key_num是类的私有变量，初始值是20，设置映射时为物理按键个数。
//        keys[key_num]
//输出：keys[key_num]--更新
uint8_t APP_TM1628::get_key(void)
{
  uint8_t i;
  enum enum_key_state key_state[20];//支持20个按键
  read_keyR();    //得最新的键状态 key_R[5]
  //将TM1628的5字节的键盘缓冲区有实际物理键盘的状态摘出来
  uint8_t line, column, j;
  for (i = 0; i < key_num; i++)   //key_num是类的私有变量，初始值是20，设置映射时为物理按键个数。 
  {
    //i为物理键盘序号-1，转换为逻辑键盘序号-1，即key_table[0]对应的是物理键号1，存的是逻辑键号，比如1
    if (key_table_P == NULL) j = i; //若按键顺序没改变
    else j = key_table_P[i] - 1;    //若按键顺序改变，并由用户程序设置了指针指向新表地址。
    line = j / 4;  //找到相对应按键寄存器的行（字节地址）
    column = j % 4;//找到相对应按键寄存器的列（字节的某位）
    if (column > 1) column += 1;
    //查看对应的按键寄存器相应位
    if ( bitRead(key_R[line], column)) key_state[i] = KEY_CLOSE;
    else  key_state[i] = KEY_OPEN;
  }
  for (i = 0; i <= key_num - 1; i++)
  {
    switch (keys[i].key_buffer)
    {
      case    NO_PRESS:
        if (key_state[i] == KEY_CLOSE)
        {
          keys[i].key_buffer = KEY_DOWN;
          keys[i].key_time = 0;
        }
        else
        {
          keys[i].key_time++;
          if (keys[i].key_time == 0xffff)   keys[i].key_time = 0xfffe;
        }
        break;
      case    KEY_DOWN:
        keys[i].key_buffer = KEY_KEEP ;
        //led_buffer[i]=i;
        //用显示器来验证回读是否有的程序
        break;
      case    KEY_KEEP:
        if (key_state[i] == KEY_OPEN)
        {
          keys[i].key_buffer = KEY_UP ;
          keys[i].key_time = 0;
        }
        else
        {
          keys[i].key_time++;
          if (keys[i].key_time == 0xffff) keys[i].key_time = 0xfffe;
        }
        break;
      case	KEY_UP:
        keys[i].key_buffer = NO_PRESS ;
        break;
        //default:;
    };//end of switch
  };//end of: for (i=0;i<=KEY_NUM-1;i++)
  return (0);
}


#define key_level1     6   //8            //达到key_level1就开始每扫描key_level1_num次就加1
#define key_level1_num    6   //8
#define key_level2  80 //150             //达到key_level2就开始每扫描key_level2_num次就加1
#define key_level2_num  3 //4
#define key_level3  150 //250
#define key_level3_num  3   //4   //用于每key_level3_num加10  


/**************关于数字加减键的处理****************/
//功能：按键操作将某数进行加减的子程序。根据按键时长进行连续加减、加速加减、十倍百倍加减。
//       对max_or_min<10进行了判断。
//输入：
//uint8_t key_No： 	从1到key_num(含)中的一个键号.key_num由类功能set_key_table()设置
//uint16_t variable： 	对variable值进行数字加减，最后返回之
//最好用指针，直接指向进行加减的变量，不用返回。
//uint8_t max_or_min:	最大或最小限值
//uint8_t ISadd： 		非0则加；为0则减
//uint8_t loop：      非0则进行循环加减数，为0不徨。暂无此功能。
//输出：返回处理过的variable
uint16_t APP_TM1628::key_add_or_sub(uint8_t key_No, uint16_t variable,
                                        uint16_t max_or_min, uint8_t ISadd, uint8_t loop)
{
  uint16_t variable1;
  variable1 = variable;
  if (keys[key_No-1].key_buffer != NO_PRESS)
  {
    if (ISadd)
    {
      if (keys[key_No-1].key_buffer == KEY_DOWN)
      {
        if (variable1 < max_or_min)
        { variable1++;
        };
      };
      if (keys[key_No-1].key_buffer == KEY_KEEP)
      {
        if (keys[key_No-1].key_time >= key_level3)
        {
          if (fmod((keys[key_No-1].key_time - key_level3), key_level3_num) == 0)
          {
            if (max_or_min > 10) //如果达不到此要求就不进行加10操作
            {
              if (variable1 <= max_or_min - 10)
              {
                variable1 = variable1 + 10;
              }
              else if (variable1 != max_or_min)
              {
                variable1 = max_or_min;
              };
            };
          };
        }
        else //不是 (key_time[key_No-1]>=key_level3)
        {
          if (keys[key_No-1].key_time >= key_level2)
          {
            if (fmod((keys[key_No-1].key_time - key_level2), key_level2_num) == 0)
            {
              if (variable1 < max_or_min)
              { variable1++;
              };
            };
          }
          else //不是(key_time[key_No]>=key_level2)
          {
            if (keys[key_No-1].key_time >= key_level1)
            {
              if (fmod((keys[key_No-1].key_time - key_level1), key_level1_num) == 0)
              {
                if (variable1 < max_or_min)
                { variable1++;
                };
              };
            };
          }//不是(key_time[key_No-1]>=key_level2)的结束
        }; //不是 (key_time[key_No-1]>=key_level3)的结束
      };//if (key_buffer[key_No-1]==KEY_KEEP)的结束
    }//加计数结束
    else //减计数
    {
      if (keys[key_No-1].key_buffer == KEY_DOWN)
      {
        if (variable1 > max_or_min)
        { variable1--;
        };
      };
      if (keys[key_No-1].key_buffer == KEY_KEEP)
      {
        if (keys[key_No-1].key_time >= key_level3)
        {
          if (fmod((keys[key_No-1].key_time - key_level3), key_level3_num) == 0)
          {
            if (variable1 >= max_or_min + 10)
            {
              variable1 = variable1 - 10;
            }
            else if (variable1 != max_or_min)
            {
              variable1 = max_or_min;
            };
          };
        }
        else //不是 (key_time[key_No-1]>=key_level3)
        {
          if (keys[key_No-1].key_time >= key_level2)
          {
            if (fmod((keys[key_No-1].key_time - key_level2), key_level2_num) == 0)
            {
              if (variable1 > max_or_min)
              { variable1--;
              };
            };
          }
          else //不是(key_time[key_No-1]>=key_level2)
          {
            if (keys[key_No-1].key_time >= key_level1)
            {
              if (fmod((keys[key_No-1].key_time - key_level1), key_level1_num) == 0)
              {
                if (variable1 > max_or_min)
                { variable1--;
                };
              };
            };
          }//不是(key_time[key_No-1]>=key_level2)的结束
        }; //不是 (key_time[key_No-1]>=key_level3)的结束
      };//if (key_buffer[key_No-1]==KEY_KEEP)的结束
    };//减计数结束
  };//if (key_buffer[key_No-1]!=NO_PRESS)的结束
  return variable1;
}

//功能：按下并保持按键时，将keys[]的键号即数组序号显示出来，从0开始
//      应用于主循环中，外部延时，延时长短影响反应快慢。
//输入：无
//输出：keys[],led_buffer[]
/*void key_test()
  {
  uint8_t i;
  get_key();
  for (i = 0; i <= KEY_NUM - 1; i++)
  {
    if (keys[i].key_buffer == KEY_KEEP) led_buffer[i] = i;
    else led_buffer[i] = 16; //  空，今后改成ASCII码识别
  }
  dis_led();
  }
*/

//功能：显示界面测试之二，在主循环之前，
//      所有显示从低位显示器led_buffer[0]开始显示012345..DIS_NUM
//      所有的指示灯从led_a0,依次亮一下,再全亮
//     led_buffer[0]对应的物理显示器由硬件驱动决定，我这里一般最右边为0。
//输入：无
//输出：led_buffer[],led_a0...led_h0
/*
  void dis_test()
  {
  uint8_t i = 0;
  led_buffer[0] = 0;
  led_buffer[1] = 1;
  led_buffer[2] = 2;
  led_buffer[3] = 3;
  led_buffer[4] = 4;
  led_buffer[5] = 5;     //如果DIS_NUM变了，修改程序

  for (i = 0; i < 8; i++)
  {
    //led_lamps_alloff();
    //LEDlamps.one_byte=1<<i;
    ledlamps_ctr(ALL, OFF);
    ledlamps_ctr(i, ON);
    dis_led();
    delay_ms(1000);
  }

  //led_lamps_allon();
  ledlamps_ctr(ALL, ON);
  dis_led();
  delay_ms(1000);
  }
*/




