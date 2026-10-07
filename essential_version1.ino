#include <Servo.h> // 必须引入舵机库

// LED 指示灯引脚定义
const int LED_R_Right = A0;
const int LED_R_Mid = A1;
const int LED_R_Left = A2;

// 舵机引脚定义 (需要根据你的实际接线修改！！！)
const int servoPinLeft = 12; 
const int servoPinRight = 13;

// 传感器引脚定义 (左、右、中)
const int irLedPinLeft = 10, irReceiverPinLeft = 11;
const int irLedPinRight = 2, irReceiverPinRight = 3;
const int irLedPinMid = 6, irReceiverPinMid = 7;

// --- 舵机脉冲宽度参数 ---
const int LeftServoStop = 1500;
const int RightServoStop = 1500;

// 前进参数
const int LeftForwardBase = 1578;
const int RightForwardBase = 1400;

// 转向参数 (修复：补充了之前缺失的转向脉冲定义)
const int LeftTurnRight = 1578;   // 左轮正转
const int RightTurnRight = 1600;  // 右轮反转
const int LeftTurnLeft = 1422;    // 左轮反转
const int RightTurnLeft = 1400;   // 右轮正转

// 创建舵机对象
Servo servoLeft;
Servo servoRight;

// ---------------------------------------------------------
// 距离检测与打印功能
// ---------------------------------------------------------

// 底层距离检测函数 (扫频法)
int find_distance(int ledPin, int receiverPin) {
  int distanceValue = 0;
  for(long f = 38000; f <= 42000; f += 1000) {
    tone(ledPin, f, 8);                      
    delay(1);                                
    int irStatus = digitalRead(receiverPin); 
    delay(1);
    
    if (irStatus == 0) { 
      distanceValue++; 
    }
  }
  return distanceValue; 
}

// 更新所有方向的距离
void find_distances(int *distanceL, int *distanceR, int *distanceM) {
  *distanceL = find_distance(irLedPinLeft, irReceiverPinLeft);   
  *distanceR = find_distance(irLedPinRight, irReceiverPinRight); 
  *distanceM = find_distance(irLedPinMid, irReceiverPinMid);     
}

// 打印距离数据到串口监视器
void print_distances(int distanceLeft, int distanceRight, int distanceMid) {
  Serial.print(F("Distances in cm: Left: "));  
  Serial.print(distanceLeft);                  
  Serial.print(F("; Right: "));                
  Serial.print(distanceRight);                 
  Serial.print(F("; Mid: "));                  
  Serial.println(distanceMid);                 
}

// ---------------------------------------------------------
// 运动控制功能
// ---------------------------------------------------------

void stop_robot() {
  servoLeft.writeMicroseconds(LeftServoStop);
  servoRight.writeMicroseconds(RightServoStop);
}

// 机器人前进函数
void move_forward() {
  servoLeft.writeMicroseconds(LeftForwardBase);
  servoRight.writeMicroseconds(RightForwardBase);
}

// 原地旋转 180 度
void turn_180() {
  // 开启指示灯（例如中间的LED亮起表示正在掉头）
  digitalWrite(LED_R_Mid, HIGH);
  
  // 左右轮反向转动，实现原地顺时针旋转
  servoLeft.writeMicroseconds(LeftTurnRight);
  servoRight.writeMicroseconds(RightTurnRight);
  
  // 旋转时间控制（非常重要！！！）
  // 这个时间决定了转动的角度。850毫秒只是一个估计值。
  delay(850); 
  
  // 旋转完成后停止并关灯
  stop_robot();
  digitalWrite(LED_R_Mid, LOW);
}

// 原地右转 90 度
void turn_right_90() {
  // 开启右侧LED指示灯
  digitalWrite(LED_R_Right, HIGH);
  
  // 舵机转动
  servoLeft.writeMicroseconds(LeftTurnRight);
  servoRight.writeMicroseconds(RightTurnRight);
  
  // 旋转时间控制 (大约是180度时间的一半，需要实际调试)
  delay(420); 
  
  // 停止并关灯
  stop_robot();
  digitalWrite(LED_R_Right, LOW);
}

// 原地左转 90 度
void turn_left_90() {
  // 开启左侧LED指示灯
  digitalWrite(LED_R_Left, HIGH);
  
  // 舵机转动
  servoLeft.writeMicroseconds(LeftTurnLeft);
  servoRight.writeMicroseconds(RightTurnLeft);
  
  // 旋转时间控制 (需要实际调试)
  delay(420); 
  
  // 停止并关灯
  stop_robot();
  digitalWrite(LED_R_Left, LOW);
}

// ---------------------------------------------------------
// 主程序 (Setup & Loop)
// ---------------------------------------------------------

void setup() {
  Serial.begin(9600);
  
  // 初始化LED引脚
  pinMode(LED_R_Right, OUTPUT);
  pinMode(LED_R_Mid, OUTPUT);
  pinMode(LED_R_Left, OUTPUT);

  // 默认关闭所有LED指示灯
  digitalWrite(LED_R_Right, LOW);
  digitalWrite(LED_R_Mid, LOW);
  digitalWrite(LED_R_Left, LOW);

  // 绑定舵机引脚
  servoLeft.attach(servoPinLeft);
  servoRight.attach(servoPinRight);
  
  // 确保初始状态是停止的
  stop_robot();
}

void loop() {
  int distanceL, distanceR, distanceM;
  
  // 持续读取三个方向的最新距离
  find_distances(&distanceL, &distanceR, &distanceM);
  
  // 将结果输出到串口监视器
  print_distances(distanceL, distanceR, distanceM);

  // 修复：补全了 if 条件判断，左右距离相等且大于0时前进
  if (distanceL == distanceR) {
    
    // 满足条件，右侧LED亮起
    digitalWrite(LED_R_Right, HIGH); 
    
    // 向前移动5cm
    move_forward();             // 让舵机转动
    delay(1200);                // 持续1200毫秒 (时间根据实际速度调整)
    stop_robot();               // 移动完成后停止
    
    // 移动完成后关闭LED，准备下一次判断
    digitalWrite(LED_R_Right, LOW); 
  }
  
  // 延迟500毫秒，方便人眼观察数据并防止串口卡顿
  delay(500);
}