#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LedControl.h>

LiquidCrystal_I2C lcd(0x3F,20,4);
LedControl lc = LedControl(12, 11, 10, 0); // (DIN, CLK, CS, ?)

static const uint8_t __attribute__((progmem)) X[] = {B01000010, B01000010, B00100100, B00011000, B00011000, B00100100, B01000010, B01000010}; // 字母 X 的logo代码，其实就是一个一个点的量灭组成的
int speedS = 8; // 贪吃蛇的初始速度
char str[25];
int score;    // 游戏得分
int xValue;   // JoyStick-X  A0
int yValue;   // JoyStick-Y  A1
int zPin = 2; // JoyStick-Z
int Enabled;  // 重启游戏使能
int FX, FY;   // 食物的坐标
int SX, SY;   // 蛇头的坐标
int KEY, K;   // 当前按键码和实际按键码
int DIN = 12;
int CS = 11;
int CLK = 10;
char s[129] = {}; // 蛇身体坐标集合，一共64个点，二维坐标128个元素 其中0没有用到 故为129个元素

void setup()
{
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  lcd.init();
  lcd.backlight();
  pinMode(zPin, INPUT);
  RANDOM();  // 获取一个随机食物坐标
  K = 0;     // 初始化按键码，实际上就是行进方向 方向初始化为0 即在首次操作摇杆前 蛇不移动
  s[1] = 2;  // x3
  s[2] = 2;  // y3
  s[3] = 1;  // x2
  s[4] = 2;  // y2
  s[5] = 0;  // x1
  s[6] = 2;  // y1 初始化蛇的身体坐标
  score = 0; // 游戏起始得分
  Logo();
  lc.clearDisplay(0);
  for (int i = 1; i <= 6; i += 2) // 从蛇头开始绘制三个点 s[1], s[3], s[5]为x坐标 s[2], s[4], s[6]为y坐标 蛇身排序： {s[5],s[6]}  {s[3],s[4]}  {s[1],s[2]} （右侧为蛇头）
  {
    lc.setLed(0, s[i], s[i + 1], true); // drawPixel(int16_t x, int16_t y, uint16_t color)
  }
  delay(400);
}
void RANDOM()
{
A:
  FX = random(0, 7);
  FY = random(0, 7);

  for (int i = 1; i <= 6 + 2 * score; i += 2) // 防止食物出现在蛇身上
  {
    if (FX == s[i] and FY == s[i + 1])
      goto A;
  }
}
void myDelay(int Time)
{ // 在蛇移动的延时期间 我们要做一些事情 比如食物的闪烁和摇杆方向的读取

  for (int t = 1; t <= Time; t++)
  {
    lc.setLed(0, FX, FY, true);
    delay(50);
    lc.setLed(0, FX, FY, false);
    delay(50);
    joyStick(); // 读取摇杆方向
  }
}
void Logo()
{
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("  Well come to  ");
  lcd.setCursor(0,1);
  lcd.print("      YCFX      ");
  lc.setLed(0, 0, 6, true); // X 右边笔画的动画
  lc.clearDisplay(0);
  delay(35);
  for (int x = 0; x <= 7; x++)
  {
    lc.setLed(0, 7 - x, x, true);
    delay(35);
  }
  for (int x = 0; x <= 7; x++)
  {
    lc.setLed(0, x, x, true);
    delay(35);
  }
  lc.clearDisplay(0);
  delay(35);
  for (int a = 1; a <= 3; a++)
  {
    for (int x = 0; x <= 7; x++)
    {
      lc.setLed(0, 7 - x, x, true);
      delay(35);
    }
    for (int x = 0; x <= 7; x++)
    {
      lc.setLed(0, x, x, true);
      delay(35);
    }
    lc.clearDisplay(0);
    delay(a);
  }
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("   Your score   ");
  lcd.setCursor(0,1);
  lcd.print("       00       ");
}
void Endelay(unsigned long duration)
{ // 字幕滚动的延时期间 我们也要做一些事情 比如判断中央键是否被按下 按下则重置游戏

  unsigned long Start = millis();
  while (millis() - Start <= duration)
  {
    if (digitalRead(zPin) == LOW)
    {
      delay(20);
      if (digitalRead(zPin) == LOW)
      {
        Enabled = 1;
      }
    }
  }
}
void joyStick()
{

  xValue = analogRead(A0); // JoyStick-X   最左值为0 最右值为1023 中间值为515
  yValue = analogRead(A1); // JoyStick-Y   最下值为0 最上值为1023 中间值为510
  if (yValue > 1000 && KEY != 8)
  {
    K = 5;
  }
  else if (yValue < 100 && KEY != 5)
  {
    K = 8;
  }
  else if (xValue < 100 && KEY != 4 && K != 0)
  { // 首次操作摇杆前 不能向左移动蛇
    K = 6;
  }
  else if (xValue > 1000 && KEY != 6)
  {
    K = 4;
  }

  if (digitalRead(zPin) == LOW) // 当按下中央键后 速度变为 2  松开后 速度恢复到当前值
    speedS = 2;
  else
    speedS = 8 - score / 2;
}

void gameover()
{ // 一旦执行到Gameover 程序停止 需要按下中央键后以重启游戏
  Enabled = 0;
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("Game Over!");
  lcd.setCursor(0,1);
  lcd.print("Your score: ");
  lcd.print(String(score));
  while (Enabled == 0)
  {
    for (int8_t x = 7; x >= -125 && Enabled == 0; x--)
    {
      lc.shutdown(0, false);
      lc.clearDisplay(0);
      lc.setIntensity(0, 8);
      Endelay(70);
    }
    for (int8_t x = 7; x >= -39 && Enabled == 0; x--)
    {
      lc.clearDisplay(0);
      Endelay(70);
    }
  }
  for (int i = 0; i < 129; i++)
  {
    s[i] = 0;
  }
  Logo();
  K = 0;
  s[1] = 2;
  s[2] = 2;
  s[3] = 1;
  s[4] = 2;
  s[5] = 0;
  s[6] = 2;
  score = 0;
}

void win()
{ // 一旦执行到Gameover 程序停止 需要按下中央键后以重启游戏
  Enabled = 0;
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("You Win!");
  lcd.setCursor(0,1);
  lcd.print("Welcome back!");
  while (Enabled == 0)
  {
    for (int8_t x = 7; x >= -125 && Enabled == 0; x--)
    {
      lc.shutdown(0, false);
      lc.clearDisplay(0);
      lc.setIntensity(0, 8);
      Endelay(70);
    }
    for (int8_t x = 7; x >= -39 && Enabled == 0; x--)
    {
      lc.clearDisplay(0);
      Endelay(70);
    }
  }
  for (int i = 0; i < 129; i++)
  {
    s[i] = 0;
  }
  Logo();
  K = 0;
  s[1] = 2;
  s[2] = 2;
  s[3] = 1;
  s[4] = 2;
  s[5] = 0;
  s[6] = 2;
  score = 0;
}

void loop()
{
  KEY = K; // 蛇每移动一次 方向才能改变一次
  if (KEY == 8) // 蛇向上运动
  {
    for (int i = 6 + 2 * score; i > 3; i = i - 2)
    {
      s[i] = s[i - 2];
      s[i - 1] = s[i - 3];
    }
    s[2] = s[2] - 1;
    if (s[2] < 0) // 超出边框的从另一边继续出现
      s[2] = 7;
  }
  else if (KEY == 5) // 蛇向下运动
  {
    for (int i = 6 + 2 * score; i > 3; i = i - 2)
    {
      s[i] = s[i - 2];
      s[i - 1] = s[i - 3];
    }
    s[2] = s[2] + 1;
    if (s[2] > 7) // 超出边框的从另一边继续出现
      s[2] = 0;
  }
  else if (KEY == 4) // 蛇向左运动
  {
    for (int i = 6 + 2 * score; i > 3; i = i - 2)
    {
      s[i] = s[i - 2];
      s[i - 1] = s[i - 3];
    }
    s[1] = s[1] - 1;
    if (s[1] < 0) // 超出边框的从另一边继续出现
      s[1] = 7;
  }
  else if (KEY == 6) // 蛇向右运动
  {
    for (int i = 6 + 2 * score; i > 3; i = i - 2)
    {
      s[i] = s[i - 2];
      s[i - 1] = s[i - 3];
    }
    s[1] = s[1] + 1;
    if (s[1] > 7) // 超出边框的从另一边继续出现
      s[1] = 0;
  }
  lc.clearDisplay(0);
  for (int i = 1; i <= 6 + 2 * score; i += 2) // 从蛇头开始绘制
  {
    lc.setLed(0, s[i], s[i + 1], true);
    delay(10);
  }
  myDelay(speedS); //（)内为延时次数 延时一次为100ms  实测200ms速度很合适 故将加速时速度设置为 2
  SX = s[1];
  SY = s[2];
  for (int i = 3; i <= 6 + 2 * score; i += 2)
  {
    if (SX == s[i] && SY == s[i + 1])
      gameover();
  }
  if (SY == FY && SX == FX)
  {
    score++;
    if (score == 64) win();
    else
    {
      lcd.setCursor(7,1);
      if (score < 10) lcd.print("0");
      lcd.print(String(score));
      RANDOM();
      if (!(score % 5))
      { // 根据得分加快蛇的速度 每吃5个食物 速度快100ms
        speedS--;
        if (speedS < 2) // 速度下限为200ms 如果速度小于200ms 速度值仍为200ms
          speedS = 2;
      }
    }
  }
}
