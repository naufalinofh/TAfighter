/*****************************************
    Send data heading n Receive data yaw,pitch
         By AAF, 2018
*****************************************/

#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "Arduino.h"
#include "Servo.h"
//#include "Stepper.h"
#define stepPin1 8 //pulse pin for controlling stepper, stepPin1
#define stepPin2 9 //pulse pin for controlling stepper, IN2
#define stepPin3 10 //pulse pin for controlling stepper, IN3
#define stepPin4 11 //pulse pin for controlling stepper, IN4

void move_stepper(double degree);
const double step_RES = 0.044 ;
String serialResponse = "";
char sz[] = "p179,y179";
boolean step_dir = HIGH; //CW is HIGH
int step_count=0;

Servo myservo1;
Servo myservo2;
int val,val2;
int prevVal,prevVal2;
float n,n2,n3;
int previous = 0;

void setup()
{
  Serial.begin(9600);              // Start serial ports
  pinMode(stepPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(stepPin3, OUTPUT);
  pinMode(stepPin4, OUTPUT);
//  stepper.setSpeed(60);
  myservo1.attach(3);
  myservo2.attach(5);
  while (!Serial) continue;
}

void loop()
{
  char *s1;
  char *s2;
  // Read serial input:
  while (Serial.available() > 0) {
    serialResponse = Serial.readStringUntil('\r\n');

    // Convert from String Object to String.
    char buf[sizeof(sz)];
    serialResponse.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *camc, *gunc,*stepc;  //string variable for cam and gun
    char *str;
    
    while ((str = strtok_r(p, ",", &p)) != NULL) // delimiter is the semicolon
      if(str[0]=='g')
      {
        camc = &str[1];
        String s=camc;
        n = s.toFloat();
    val = map(n, 0, 180, 0, 180);
    if (val != prevVal)
    {
      myservo1.write((-1*val2)+90);
      prevVal = val;
    }  
      }
    
    else if(str[0]=='t'){
        gunc = &str[1];
        String s=gunc;
        n2 = s.toFloat();
        //myservo2.write(n2);
        val2 = map(n2, 0, 180, 0, 180);
    if (val2 != prevVal2)
    {
      myservo2.write((-1*val2)+90);
      prevVal2 = val2;
    }
     //str = strtok_r(p, ",", &p);  
  }
   
    else if(str[0]=='c'){
        stepc = &str[1];
        String s=stepc;
        n3 = s.toFloat();
        if(n3 != previous) //if the value change
  {
    move_stepper(n3);
  }
      previous=n3;
      }
    str = strtok_r(p, ",", &p);
  }

}

void stepper(){
  switch(step_count){
     case 0:
       digitalWrite(stepPin1, LOW); 
       digitalWrite(stepPin2, LOW);
       digitalWrite(stepPin3, LOW);
       digitalWrite(stepPin4, HIGH);
     break; 
     case 1:
       digitalWrite(stepPin1, LOW); 
       digitalWrite(stepPin2, LOW);
       digitalWrite(stepPin3, HIGH);
       digitalWrite(stepPin4, HIGH);
     break; 
     case 2:
       digitalWrite(stepPin1, LOW); 
       digitalWrite(stepPin2, LOW);
       digitalWrite(stepPin3, HIGH);
       digitalWrite(stepPin4, LOW);
     break; 
     case 3:
       digitalWrite(stepPin1, LOW); 
       digitalWrite(stepPin2, HIGH);
       digitalWrite(stepPin3, HIGH);
       digitalWrite(stepPin4, LOW);
     break; 
     case 4:
       digitalWrite(stepPin1, LOW); 
       digitalWrite(stepPin2, HIGH);
       digitalWrite(stepPin3, LOW);
       digitalWrite(stepPin4, LOW);
     break; 
     case 5:
       digitalWrite(stepPin1, HIGH); 
       digitalWrite(stepPin2, HIGH);
       digitalWrite(stepPin3, LOW);
       digitalWrite(stepPin4, LOW);
     break; 
       case 6:
       digitalWrite(stepPin1, HIGH); 
       digitalWrite(stepPin2, LOW);
       digitalWrite(stepPin3, LOW);
       digitalWrite(stepPin4, LOW);
     break; 
     case 7:
       digitalWrite(stepPin1, HIGH); 
       digitalWrite(stepPin2, LOW);
       digitalWrite(stepPin3, LOW);
       digitalWrite(stepPin4, HIGH);
     break; 
     default:
       digitalWrite(stepPin1, LOW); 
       digitalWrite(stepPin2, LOW);
       digitalWrite(stepPin3, LOW);
       digitalWrite(stepPin4, LOW);
     break; 
    }
    stepper_next();
}

void stepper_next(){
  if(step_dir==HIGH){ step_count++;}
  if(step_dir==LOW){ step_count--; }
  if(step_count>7){step_count=0;}
  if(step_count<0){step_count=7; }
}

void move_stepper(double degree){
  Serial.println("move stepper");
  double deg = degree;
    
  //clockwise is positive
  if (degree > 0)
  {
    step_dir=HIGH;
  }else
  {
    //digitalWrite(dirPin,LOW);
    step_dir=LOW;
    deg = -deg;
  }
  Serial.print("degree =");
  Serial.print(deg);
  if (degree!=0)
  {
    float dump = deg/step_RES;
    int step_req = (int) dump;
    int steps_left = step_req;
    //while (steps_left>0){
    Serial.print("steps req =");
  Serial.print(step_req);
  
    for(int x = 0; x <= step_req; x++) {  //give pulse until degree achieved
      //currentMillis = micros();
      delay(1); //frequensy of pulse to move stepper. Config based on datasheet
      //if(currentMillis-last_time>=1000){
      stepper(); 
        //last_time=micros();
        //steps_left--;
       //} 
    } 
    Serial.print(step_req);
  }
  
  n3 +=degree;
  previous = n3;
  delay(10);
}



