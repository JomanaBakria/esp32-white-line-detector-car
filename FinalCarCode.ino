#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#define SPEED_FORWARD_FL  250
#define SPEED_FORWARD_FR  244
#define SPEED_FORWARD_BL  250
#define SPEED_FORWARD_BR  244
#define SPEED_BACK_FL     245   
#define SPEED_BACK_FR     238
#define SPEED_BACK_BL     250
#define SPEED_BACK_BR     250

#define LIGHT_SENSOR_PIN   34
#define LIGHT_SENSOR_PIN2  35
#define THRESHOLD1         3500
#define THRESHOLD2         1100

#define FL_IN1 18
#define FL_IN2 19
#define FR_IN3 14
#define FR_IN4 27
#define BL_IN3 26
#define BL_IN4 25
#define BR_IN1 33
#define BR_IN2 32
#define ENA_FL 5
#define ENB_BL 4
#define ENB_FR 16
#define ENA_BR 17

#define CH_FL 0
#define CH_FR 1
#define CH_BL 2
#define CH_BR 3

const unsigned long OFF_MS        = 1300;   
const unsigned long ON_MS         = 1700;   
const unsigned long SAMPLE_MS     =   10;   
const unsigned long LINE_PAUSE    =  400;   
const unsigned long BACKWARD_MS   = 3600;   

bool  driving            = false;  
bool  goingBackward      = false;
bool  waitingLine        = false;
bool  inWhiteLine        = false;
bool  inSampleWindowFwd  = false;
bool  backwardReady      = false;   
bool backwardwindow =false;
      
unsigned long windowTimer    = 0;
unsigned long sampleTimer    = 0;
unsigned long linePauseStart = 0;
unsigned long backwardStart  = 0;

int   whiteLineCount     = 0;
//define CHANNEL frequency&resolution
void setupPWM() {
  const int f=5000, r=8;
  ledcSetup(CH_FL,f,r); ledcSetup(CH_FR,f,r);
  ledcSetup(CH_BL,f,r); ledcSetup(CH_BR,f,r);
  ledcAttachPin(ENA_FL,CH_FL); ledcAttachPin(ENB_FR,CH_FR);
  ledcAttachPin(ENB_BL,CH_BL); ledcAttachPin(ENA_BR,CH_BR);
}
//function for moving forward
void moveForward(int sFL,int sFR,int sBL,int sBR){
  ledcWrite(CH_FL,sFL); ledcWrite(CH_FR,sFR);
  ledcWrite(CH_BL,sBL); ledcWrite(CH_BR,sBR);
  digitalWrite(FL_IN1,HIGH); digitalWrite(FL_IN2,LOW);
  digitalWrite(FR_IN3,LOW);  digitalWrite(FR_IN4,HIGH);
  digitalWrite(BL_IN3,HIGH); digitalWrite(BL_IN4,LOW);
  digitalWrite(BR_IN1,LOW);  digitalWrite(BR_IN2,HIGH);
}
//function for moving backward
void moveBackward(int sFL,int sFR,int sBL,int sBR){
  ledcWrite(CH_FL,sFL); ledcWrite(CH_FR,sFR);
  ledcWrite(CH_BL,sBL); ledcWrite(CH_BR,sBR);
  digitalWrite(FL_IN1,LOW);  digitalWrite(FL_IN2,HIGH);
  digitalWrite(FR_IN3,HIGH); digitalWrite(FR_IN4,LOW);
  digitalWrite(BL_IN3,LOW);  digitalWrite(BL_IN4,HIGH);
  digitalWrite(BR_IN1,HIGH); digitalWrite(BR_IN2,LOW);
}
//function for stop car
void stopCar(){
  ledcWrite(CH_FL,0); ledcWrite(CH_FR,0);
  ledcWrite(CH_BL,0); ledcWrite(CH_BR,0);
  digitalWrite(FL_IN1,LOW); digitalWrite(FL_IN2,LOW);
  digitalWrite(FR_IN3,LOW); digitalWrite(FR_IN4,LOW);
  digitalWrite(BL_IN3,LOW); digitalWrite(BL_IN4,LOW);
  digitalWrite(BR_IN1,LOW); digitalWrite(BR_IN2,LOW);
}

void activeBrake(bool reverse = true, unsigned long ms = 60, uint8_t pwm = 200){
  if(reverse){
    ledcWrite(CH_FL,pwm); ledcWrite(CH_FR,pwm);
    ledcWrite(CH_BL,pwm); ledcWrite(CH_BR,pwm);
    digitalWrite(FL_IN1,LOW);  digitalWrite(FL_IN2,HIGH);
    digitalWrite(FR_IN3,HIGH); digitalWrite(FR_IN4,LOW);
    digitalWrite(BL_IN3,LOW);  digitalWrite(BL_IN4,HIGH);
    digitalWrite(BR_IN1,HIGH); digitalWrite(BR_IN2,LOW);
  }else{
    ledcWrite(CH_FL,pwm); ledcWrite(CH_FR,pwm);
    ledcWrite(CH_BL,pwm); ledcWrite(CH_BR,pwm);
    digitalWrite(FL_IN1,HIGH); digitalWrite(FL_IN2,LOW);
    digitalWrite(FR_IN3,LOW);  digitalWrite(FR_IN4,HIGH);
    digitalWrite(BL_IN3,HIGH); digitalWrite(BL_IN4,LOW);
    digitalWrite(BR_IN1,LOW);  digitalWrite(BR_IN2,HIGH);
  }
  delay(ms);
  stopCar();
}

void setup(){
  Serial.begin(115200); // Initialize baudrate of UART
  SerialBT.begin("RJ CAR");
  pinMode(FL_IN1,OUTPUT); pinMode(FL_IN2,OUTPUT);
  pinMode(FR_IN3,OUTPUT); pinMode(FR_IN4,OUTPUT);
  pinMode(BL_IN3,OUTPUT); pinMode(BL_IN4,OUTPUT);
  pinMode(BR_IN1,OUTPUT); pinMode(BR_IN2,OUTPUT);
  pinMode(LIGHT_SENSOR_PIN,INPUT);
  pinMode(LIGHT_SENSOR_PIN2,INPUT);
  setupPWM();
}

void loop(){
  if(SerialBT.available()){  // Check if data is available from Bluetooth and act accordingly
    String cmd = SerialBT.readStringUntil('\n');  
    cmd.trim();   //Remove space
    if(cmd=="1"){
      driving=true;
      goingBackward=false;
      waitingLine=false;
      inWhiteLine=false; 
      backwardReady=false;
      moveForward(SPEED_FORWARD_FL,SPEED_FORWARD_FR,SPEED_FORWARD_BL,SPEED_FORWARD_BR);
      inSampleWindowFwd=false; 
      windowTimer=millis(); //save starting time in windowtimer
      SerialBT.println("moving forward");
    }
    else if(cmd=="0"){
      driving=false;
      goingBackward=false; 
      stopCar();
      SerialBT.println("stop");
    }
  }

  unsigned long now = millis(); 

  if(driving && !goingBackward && !waitingLine){
    if(!inSampleWindowFwd && now - windowTimer >= OFF_MS){ //Activate sampling window
      inSampleWindowFwd=true;  
      windowTimer=now;
    }else if(inSampleWindowFwd && now - windowTimer >= ON_MS){
      inSampleWindowFwd=false; 
      windowTimer=now;
    }

    if(inSampleWindowFwd && now - sampleTimer >= SAMPLE_MS){ //Sampling every 10 milliseconds
      sampleTimer=now;
      int v1 = analogRead(LIGHT_SENSOR_PIN);
      int v2 = analogRead(LIGHT_SENSOR_PIN2);
      SerialBT.print("sensor1: ");
      SerialBT.print(v1);
      SerialBT.print(" | sensor2: ");
      SerialBT.println(v2);
      if(!inWhiteLine && v1>THRESHOLD1 && v2>THRESHOLD2){ //if white line detected in forward
        activeBrake(true);
        waitingLine=true; 
        linePauseStart=now;
        inWhiteLine=true;
        SerialBT.println("White line detected");
      }
      if(v1<THRESHOLD1 && v2<THRESHOLD2) inWhiteLine=false;
    }
  }
  

  if(waitingLine && now - linePauseStart >= LINE_PAUSE){ //White line stop time ended
    waitingLine=false; 
    whiteLineCount++;
    inSampleWindowFwd=false; 
    windowTimer=now;

    if(whiteLineCount>=3){
      SerialBT.println("Time for going backward");
      moveBackward(SPEED_BACK_FL,SPEED_BACK_FR,SPEED_BACK_BL,SPEED_BACK_BR);
      goingBackward=true; 
      backwardStart=now; 
      backwardReady=false;
    }else{
      moveForward(SPEED_FORWARD_FL,SPEED_FORWARD_FR,SPEED_FORWARD_BL,SPEED_FORWARD_BR);
    }
  }

  if(goingBackward){
    if(!backwardReady && now - backwardStart >= BACKWARD_MS){     //time to activate sampling window
      backwardReady=true;
      SerialBT.println("מתחיל לזהות פס לאחור");
    }

    if(backwardReady && now - sampleTimer >= SAMPLE_MS){    //Sampling every 10 milliseconds
      sampleTimer=now;
      int v1 = analogRead(LIGHT_SENSOR_PIN);
      int v2 = analogRead(LIGHT_SENSOR_PIN2);
      SerialBT.print("sensor1: ");
      SerialBT.print(v1);
      SerialBT.print(" | sensor2: ");
      SerialBT.println(v2);

      if(!inWhiteLine && v1>THRESHOLD1 && v2>THRESHOLD2 && !backwardwindow){ //if white line detected in backward
        activeBrake(false);    
        SerialBT.println("White line detected");
        delay(LINE_PAUSE);             
        backwardwindow=true;            
        //moveBackward(SPEED_BACK_FL,SPEED_BACK_FR,SPEED_BACK_BL,SPEED_BACK_BR);
      }

      if(v1<THRESHOLD1 && v2<THRESHOLD2) inWhiteLine=false;
    }
  }
}

