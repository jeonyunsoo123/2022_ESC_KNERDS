#include <Adafruit_NeoPixel.h> // Neopixel을 사용하기 위해서 라이브러리를 불러옵니다.

#include <avr/power.h>            


#define NeoPixel_PIN 6       // 네오픽셀 LED 제어 입력 핀 설정
#define NUMPIXELS 120        // Neopixel LED 소자 수
#define TunnelPIXELS 56


#define BRIGHTNESS 255     // 밝기 설정 0(어둡게) ~ 255(밝게) 까지 임의로 설정 가능

#define MAX_ANALOGE 900      // 센서에 물체가 인식되었다고 판단하는 최대 아날로그 값
#define MIN_ANALOGE 500      // 센서에 물체가 인식되었다고 판단하는 최소 아날로그 값

#define Accident 400        // 동일한 센서에 일정 시간 이상 감지시 사고로 판단하는 시간
#define Wait 100             // Loop 한번 돌고 나서 기다리는 시간
#define Congest 4            // 정체라고 판단하는 센서 수

int Sensor_on_count = 0;     //현재 Loop에서 물체가 인식된 센서 수
int left_Sensor_on_count = 0;//현재 Loop에서 터널에 들어가는 방향 기준 왼쪽에 위치한 센서 중 물체에 인식된 센서 수
int right_Sensor_on_count = 0;//현재 Loop에서 터널에 들어가는 방향 기준 오른쪽에 위치한 센서 중 물체에 인식된 센서 수

int Sensor_on_timer[12] = {0,0,0,0,0,0,0,0,0,0,0,0};  // 각각의 센서에 물체가 연속적으로 인식되어 있는 시간을 저장해둔 배열  
int Sensor_on_timer_Max = 0; //Sensor_on_timer의 Max 값을 저장해 두기 위한 변수


char *Analog_Sensors[] = {A0, A1, A2, A3, A4, A5, 
                          A6, A7, A8, A9, A10, A11
                         };


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NeoPixel_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  strip.setBrightness(BRIGHTNESS);    //  BRIGHTNESS 만큼 밝기 설정 
  strip.begin();                                        //  Neopixel 제어를 시작
  strip.show();                                        //  Neopixel 동작 초기화 합니다
}


//////////////////////////////Loop///////////////////////////////////////////////////

void loop() {
  Sensor_on_count = 0;
  left_Sensor_on_count = 0;
  right_Sensor_on_count = 0;

  count_active_sensor();       //현재 물체가 인식되고 있는 센서의 수를 Sensor_on_count 변수에 저장
  count_left_active_sensor();
  count_right_active_sensor();


  sensor_on_time_check();   //현재 센서에 물체가 연속적으로 인식되고 있는 각각의 시간을 확인해서 Sensor_on_timer[12] 배열에 저장

  //----------------------------------Left----------------------------------------------//
  if(left_Sensor_on_count >= Congest){           //도로 왼쪽에 위치한 센서 중 4개 이상의 센서에 감지가 될 경우 정체로 판단
    setLeftYellow();
    reset_count();
    Serial.println("1");
  }

  else if(find_left_MAX() >= Accident){    //도로 왼쪽에 위치해 있는 특정 센서에 차량이 일정 시간 이상 지속적으로 감지될 경우 사고로 판단
    setLeftRed();
    Serial.println("2");
  }

  else{
    setLeftGreen();
  }

  
  //-----------------------------------Right----------------------------------------------//
  if(right_Sensor_on_count >= Congest){           //도로 오른쪽에 위치한 센서 중 3개 이상의 센서에 감지가 될 경우 정체로 판단
    setRightYellow();
    reset_count();
    Serial.println("1");
  }

  else if(find_right_MAX() >= Accident){    //도로 오른쪽에 위치해 있는 특정 센서에 차량이 일정 시간 이상 지속적으로 감지될 경우 사고로 판단
    setRightRed();
    Serial.println("2");
  }

  else{
    setRightGreen();
  }

  //--------------------------------------------------------------------------------------//

  delay(Wait);
}

//--------------------------------------------LOOP END----------------------------------------------------------//



//-------------------------------------------Loop 문에서 사용하기 위한 함수들-------------------------------------//
void setGreen(){
  colorWipe(strip.Color(0,255,0),0);
}

void setLeftGreen(){
  leftcolorWipe(strip.Color(0,255,0),0);
  leftroadcolorWipe(strip.Color(0,255,0), 0);
}

void setRightGreen(){
  rightcolorWipe(strip.Color(0,255,0),0);
  rightroadcolorWipe(strip.Color(0,255,0), 0);
}



void setYellow(){
  colorWipe(strip.Color(255,255,0),0);
}

void setLeftYellow(){
  leftcolorWipe(strip.Color(255,255,0),0);
  leftroadcolorWipe(strip.Color(255,255,0), 0);
}

void setRightYellow(){
  rightcolorWipe(strip.Color(255,255,0),0);
  rightroadcolorWipe(strip.Color(255,255,0), 0);
}



void setRed(){
  colorWipe(strip.Color(255,0,0),0);
}

void setLeftRed(){
  leftcolorWipe(strip.Color(255,0,0),0);
  leftroadcolorWipe(strip.Color(255,0,0), 0);
}

void setRightRed(){
  rightcolorWipe(strip.Color(255,0,0),0);
  rightroadcolorWipe(strip.Color(255,0,0), 0);
}

void colorWipe(uint32_t c, uint8_t wait){    //  loop에서 사용할 colorWipe 함수를 만든 공식
  for(uint16_t i=0; i<TunnelPIXELS; i++){
  strip.setPixelColor(i,c);
  strip.show();
  delay(wait);
  }
}

void leftcolorWipe(uint32_t c, uint8_t wait){  
  for(uint16_t i=0; i<TunnelPIXELS; i++){
    if ((i / 7) % 2 == 1)  continue;
  strip.setPixelColor(i,c);
  strip.show();
  delay(wait);
  }
}

void leftroadcolorWipe(uint32_t c, uint8_t wait){  
  for(uint16_t i= TunnelPIXELS; i< (TunnelPIXELS + 24); i++){
  if(((i - TunnelPIXELS) / 6) % 2 == 1)   
    continue;
  strip.setPixelColor(i,c);
  strip.show();
  delay(wait);
  }
}

void rightcolorWipe(uint32_t c, uint8_t wait){
  for(uint16_t i= 0; i<TunnelPIXELS; i++){
  if((i / 7) % 2 == 0)  
    continue;
  strip.setPixelColor(i,c);
  strip.show();
  delay(wait);
  }
}

void rightroadcolorWipe(uint32_t c, uint8_t wait){  
  for(uint16_t i= TunnelPIXELS; i<(TunnelPIXELS + 24); i++){
  if(((i - TunnelPIXELS) / 6) % 2 == 0)  continue;
  strip.setPixelColor(i,c);
  strip.show();
  delay(wait);
  }
}


void count_active_sensor(){                    //물체가 인식된 센서의 개수 구하는 함수
  for(int i = 0; i < 12 ; i++){
    if(analogRead(Analog_Sensors[i]) < MAX_ANALOGE && (analogRead(Analog_Sensors[i]) > MIN_ANALOGE)){
      Sensor_on_count++;
    }
  }
}

void count_left_active_sensor(){              //왼쪽 센서에 물체가 인식된 센서 수
  for(int i = 1; i < 12; i += 2){
    if(analogRead(Analog_Sensors[i]) < MAX_ANALOGE && (analogRead(Analog_Sensors[i]) > MIN_ANALOGE)){
      left_Sensor_on_count++;
    }
  }
}

void count_right_active_sensor(){             //오른쪽 센서에 물체가 인식된 센서 수
  for(int i = 0; i < 12; i += 2){
    if(analogRead(Analog_Sensors[i]) < MAX_ANALOGE && (analogRead(Analog_Sensors[i]) > MIN_ANALOGE)){
      right_Sensor_on_count++;
    }
  }
}



void sensor_on_time_check(){
  for(int i = 0; i < 12; i++){
    if(analogRead(Analog_Sensors[i]) < MAX_ANALOGE && (analogRead(Analog_Sensors[i]) > MIN_ANALOGE)){   //만약 특정 센서에 물체가 인식되어 있으면 해당하는 센서 index와 동일한 Sensor_on_timer 배열의 인덱스를 Wait 만큼 증가 (사고 판단에 사용)
      Sensor_on_timer[i] += Wait;
    }
    else{
      Sensor_on_timer[i] = 0;
    }
  }
}

void reset_count(){
  for(int i = 0; i < 12 ; i++){
    Sensor_on_timer[i] = 0;
  }
}


int find_MAX(){                                                                              //Sensor_on_timer 배열의 최댓값을 리턴하는 함수
    int max = Sensor_on_timer[0];
    for (int i = 0; i < 12; i++) {
      if (Sensor_on_timer[i] > max) max = Sensor_on_timer[i];
    }
    return max;
}

int find_left_MAX(){
  int max = Sensor_on_timer[1];
  for (int i = 1; i < 12; i += 2) {
    if (Sensor_on_timer[i] > max) max = Sensor_on_timer[i];
  }
  return max;
}

int find_right_MAX(){
  int max = Sensor_on_timer[2];
  for (int i = 0; i < 12; i += 2) {
    if (Sensor_on_timer[i] > max) max = Sensor_on_timer[i];
  }
  return max;
}


//---------------------------------------------검증용 함수-------------------------------------------//

void print_sensor_on_time(){                                                               //Sensor_on_timer 배열 출력
  for(int i = 0; i < 12; i++){
    Serial.print(Sensor_on_timer[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void print_Analog_Value(){                                                                //각각의 센서의 Analog 값 출력
  for(int i = 0; i < 12; i++){
    Serial.print(Analog_Sensors[i]);
    Serial.print(" : ");
    Serial.print(analogRead(Analog_Sensors[i]));
    Serial.print("  ");
  }
  Serial.println("\n");
}

