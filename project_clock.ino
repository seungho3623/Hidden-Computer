/*
 * TRIG PIN : 5
 * ECHO PIN : 18
 * DHT22 PIN : 19
 * SSD1306
 * SCL PIN : 22
 * SDA PIN : 21
*/

#include <Adafruit_SSD1306.h>

#include <Adafruit_GFX.h>

#include <WiFi.h>  
#include <DHT.h>
#include <Wire.h>
#include <SPI.h>

#include "time.h" 

#include <BleKeyboard.h>

#define DHTPIN 19
#define DHTTYPE DHT22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1            // -1은 아두이노 보드 리셋과 연동 됨.
#define SCREEN_ADDRESS 0x3C 

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2                                                                                                                                                                                

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

#define Timer_x  30   // 시계 중심축 좌표
#define Timer_y  display.height()/2  //시계 중심축 좌표


#define TRIG 5
#define ECHO 18
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DHT dht(DHTPIN, DHTTYPE);
BleKeyboard blekeyboard;

int humidity;
int temperature;

const char* ssid       = "HiToby";        // 공유기 SSID
                                          //YOUR_SSID
const char* password   = "11111111";      // 공유기 비밀번호
                                          //YOUR_PASSWORD

const char* ntpServer = "pool.ntp.org";     // NTP 서버
uint8_t timeZone = 9;                       // 한국 타임존 설정
uint8_t summerTime = 0; // 3600             // 썸머타임 시간

int s_hh = 12;         // 시간 설정용 변수 및 초기값
int s_mm = 59;          
uint8_t s_ss = 45;     
uint16_t s_yy = 2017;   
uint8_t s_MM = 11;     
uint8_t s_dd = 19;     

time_t now;          // 현재 시간 변수 
time_t prevEpoch;    // 이전 UTC 시간 변수
struct tm timeinfo;  // 로컬 시간 반영용 구조체 변수 선언

void init_SSD1306()
{
  // OLED 초기화
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); 
  
  display.clearDisplay();  // 디스플레이 버퍼 내용 클리어
  display.display();       // 디스플레이 버퍼 내용 출력
 
  display.setTextColor(WHITE, BLACK);
}

void init_WiFi()
{
  Serial.printf("Connecting to %s ", ssid); // 공유기 아이디 표시
  WiFi.begin(ssid, password);               // 와이파이 시작 및 연결
  while (WiFi.status() != WL_CONNECTED) {   // 와이파이 연결이 안된 상태이면 
    delay(500);                             
    Serial.print(".");                      // '.' 표시
  }
  Serial.println(" CONNECTED");
  get_NTP();                                // NTP 서버 동기화
}


void init_BLE()
{
  Serial.println("Waiting BLE!"); 
  blekeyboard.begin(); //BLE Keyboard 시작

  //BLE 연결 될 때까지 반복
  while(!blekeyboard.isConnected())
  {
    //BLE 연결 확인
    Serial.println("Starting BLE!");
  }
}

void init_HC_SR04()
{
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
}

void get_NTP()  // NTP 서버 동기화    
{      
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer); // NTP 서버 접속 및 동기화
  while(!getLocalTime(&timeinfo)){            // 로컬 시간 반영함수이 리턴 값이 false이면, 시간값 없으면
    Serial.println("Failed to obtain time");  // 메시지 표시
    set_time();                               // 초기값으로 시간 설정
    return;
  }
}

void set_time()     // 사용자 시간 설정 함수
{   
  struct tm tm;     // 사용자 시간 설정용 구조체 변수 선언
  tm.tm_year = s_yy - 1900; 
  tm.tm_mon = s_MM - 1;
  tm.tm_mday = s_dd;
  tm.tm_hour = s_hh;
  tm.tm_min = s_mm;
  tm.tm_sec = s_ss;
  time_t t = mktime(&tm);    // UTC 값으로 변환
  printf("Setting time: %s", asctime(&tm)); // 설정 시간을 문자열로 출력
  struct timeval now = { .tv_sec = t };     // 설정시간을 현재 시간에 반영 
  settimeofday(&now, NULL); 
}

void printLocalTime()
{
  if (time(&now) != prevEpoch) 
  {   // 현재 UTC 시간 값과 이전 UTC 시간 값이 다르면 
    //Serial.println(time(&now));  // 현재 UTC 시간 값 출력
    getLocalTime(&timeinfo);       // 로컬 변경함수이용 UTC 시간값 변경
    int dd = timeinfo.tm_mday;     // 구조체 내 해당값 가져오기
    int MM = timeinfo.tm_mon + 1;
    int yy = timeinfo.tm_year +1900;
    int ss = timeinfo.tm_sec;
    int mm = timeinfo.tm_min;
    int hh = timeinfo.tm_hour;
    
    Serial.print(yy); Serial.print(". ");
    Serial.print(MM); Serial.print(". ");
    Serial.print(dd); Serial.print(" ");
    Serial.print(hh); Serial.print(": ");
    Serial.print(mm); Serial.print(": ");
    Serial.println(ss);
    prevEpoch = time(&now); // 현재 UTC 시간 값을 저장하여 1초마다 실행되도록 함.
  }
}

void get_DHT()
{
  humidity = dht.readHumidity();        // DHT 센서에서 습도데이터 읽기
  temperature = dht.readTemperature();  // DHT 센서에서 온도데이터 읽기

  
  Serial.print("humidity:"); 
  Serial.println(humidity);             // 습도출력

  Serial.print("temperature:");
  Serial.println(temperature);          // 온도출력
  delay(1000);
}

void printDigits(int digits)
{ 
   if(digits < 10){
     display.print('0');
     display.print(digits);
   }else
     display.print(digits);
}

void draw_text(byte x_pos, byte y_pos, char *text, byte text_size) {
  display.setCursor(x_pos, y_pos);
  display.setTextSize(text_size);
  display.print(text);
  display.display();
}

void draw_min(int Min)
{
   Min = Min * 6 + 270;  //분 단위를 각도로 변환(1분은 6도)
   if(Min > 360)
      Min = Min -360; 
   
   float rad = 3.14159 /180 * Min;     //라디안으로 변환
   float x = 27 * cos(rad) + Timer_x;  // 시계 기준축에서 27픽셀만큼 벗어난 분 위치 계산
   float y = 27 * sin(rad) + Timer_y;
   display.drawLine(Timer_x, Timer_y, x , y, WHITE); // 중심축에서 분 위치 까지 라인 그리기
}

void draw_hour(float Hour,  float Min)
{ 
   Hour = (Hour * 30 + 270) + (6*Min/60);  //시간 단위를 각도로 변환(1시간은 30도, 0~59분 까지 지날때 시침이 조금씩 이동함)
   if(Hour > 360)
      Hour = Hour -360;
  
   float rad = 3.14159 /180 * Hour;
   float x = 15 * cos(rad) + Timer_x; //중심축으로부터 15픽셀 (시침은 분침보다 작게 그림)
   float y = 15 * sin(rad) + Timer_y;
   display.drawLine(Timer_x, Timer_y, x , y, WHITE); //
}

void draw_second(float Second)
{
   Second = (Second * 6 + 270) ;  //초 단위를 각도로 변환
   if(Second > 360)
      Second = Second -360;
  
   float rad = 3.14159 /180 * Second;
   float x = 15 * cos(rad) + Timer_x;
   float y = 15 * sin(rad) + Timer_y;
   display.drawCircle(x, y,  1, WHITE); // 초침은 작은 원으로 표시
}

void write_date(int Month, int Day)
{
   display.setTextSize(1);
   display.setCursor(65,0); //픽셀단위
   display.setTextColor(WHITE);
   printDigits(Month);
   display.print("/");
   printDigits(Day); 
}

void write_time(int Hour, int Minute, int Second)
{
   String Second_mark;
   if(Second%2 == 0) //초 나타낼때 깜빡임
      Second_mark = ":";
   else
      Second_mark = " ";
  
   display.setTextSize(2);
   display.setCursor(65,10); //픽셀단위
   display.setTextColor(WHITE);
   printDigits(Hour);
   display.print(Second_mark);
   printDigits(Minute); 
}

void write_Humidity(void)
{
   get_DHT();

   byte w_value;

   display.setTextSize(1);
   display.setCursor(65,34); //픽셀단위

   display.print("Humi ");
   display.setTextSize(2);
   display.print(byte(humidity));
  
   display.setCursor(65,50); //픽셀단위
   display.setTextSize(1);
   display.print("Temp ");
   display.setTextSize(2);
   display.print(byte(temperature));
}

void display_SSD1306()
{
  display.clearDisplay();
  display.drawCircle(Timer_x, Timer_y,  30, WHITE); // 시계 테두리 

  getLocalTime(&timeinfo);     // 로컬 변경함수이용 UTC 시간값 변경
  int dd = timeinfo.tm_mday;       // 구조체 내 해당값 가져오기
  int MM = timeinfo.tm_mon + 1;
  int yy = timeinfo.tm_year +1900;
  int ss = timeinfo.tm_sec;
  int mm = timeinfo.tm_min;
  int hh = timeinfo.tm_hour;
   
  draw_hour(hh, mm); //시
  draw_min(mm); //분
  draw_second(ss); //초

  write_date(MM, dd);
  write_time(hh, mm, ss);

  write_Humidity();
   
  display.display();
}

void window_conversion()
{
  /*
  float volt = (float)map(analogRead(Distance_Pin), 0, 1023, 0, 5000); //측정 전압 변환
  float distance = 60.594 * pow(volt, -1.1904); //거리 계산
  */
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  int duration = pulseIn(ECHO, HIGH);
  
  float distanceCm = duration * SOUND_SPEED/2;
  
  float distanceInch = distanceCm * CM_TO_INCH;
  
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  
  if((int)distanceCm < 10)
  {
    //ctrl + win + -> 
    //오른쪽 윈도우 변환
    blekeyboard.press(KEY_LEFT_GUI);
    blekeyboard.press(KEY_LEFT_CTRL);
    blekeyboard.press(KEY_RIGHT_ARROW);
    blekeyboard.release(KEY_LEFT_GUI);
    blekeyboard.release(KEY_LEFT_CTRL);
    blekeyboard.release(KEY_RIGHT_ARROW);
  }
}

void setup() 
{
  Serial.begin(115200);
  init_SSD1306();
  dht.begin();
  init_WiFi();
  init_BLE();
  init_HC_SR04();
}

void loop() 
{
  if(Serial.available() > 0)
  {
    String temp = Serial.readStringUntil('\n');
    if (temp == "1") set_time();     // set time
    else if (temp == "2") get_NTP(); // NTP Sync
  }
  printLocalTime();                  // 로컬 시간 출력
  display_SSD1306();
  window_conversion();
}
