/*     Turret Control
 *      Created by: Naufalino Fadel Hutomo
 *      
 */
//Library
#include <Servo.h>
#include <string.h>
#include <Wire.h>

//#define servoCAM 3 //digital pin for camera pan setpoint signal
#define servoTILT 5 //digital pin for camera tilt setpoint signal
#define servoGUN 6 //servo for control gun
#define stepPin 3 //pulse pin for controlliong turret stepper
#define dirPin 4 //pulse pin for controlliong turret stepper
#define stepPin1 8 //pulse pin for controlling stepper, stepPin1
#define stepPin2 9 //pulse pin for controlling stepper, IN2
#define stepPin3 10 //pulse pin for controlling stepper, IN3
#define stepPin4 11 //pulse pin for controlling stepper, IN4
#define compassPin A3// pin to control which compass is active, low for compass_cam, high for compass_turret

//Function Definition
void get_setpoint();
void move_cam(float degree);
void move_tilt(float degree);
void move_stepper(float degree);
void move_gun(float degree);
void move_all();

//Servo servo_cam;  // servo object for camera yaw
Servo servo_tilt;  // servo object for camera yaw
Servo servo_gun; // // servo object for gun pitch


//CONSTANT
const float step_RES = 0.044 ; // step resolution is 360/64 step (from motor) / 64 (gearbox)/2 (gear turret). If you want to change the resolution, change the driver configuration. Driver configuration 010 -> quarter step
const float yaw_RES = 1.8/4 ; //nema 17 resolution is 1.8 degree, with gear ratio between motor and turret is 1:4

String readStr;
float cam_set = 0;  //for camera pan set condition
float cam_prev = 0;
float cam_act=0, cam_act_init=0;  //heading measurement of camera
float cam_sum=0, cam_last_pid=0; // for PID calculation of camera movement
int cam_stepState;

float tilt_set = 0;  //for camera tilt set condition
float tilt_prev = 0;
float yaw_set = 0;   //for turret yaw set condition
float yaw_act=0;  //heading measurement of yaw

float gun_set = 0;   //for gun pitch set condition
float gun_prev = 0;

bool step_dir = HIGH; //CW is HIGH


void setup() {
  // Sets pins Mode
  //pinMode(stepPin,OUTPUT); 
  pinMode(stepPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(stepPin3, OUTPUT);
  pinMode(stepPin4, OUTPUT);
  //servo_cam.attach(servoCAM);
  servo_tilt.attach(servoTILT);
  servo_gun.attach(servoGUN);
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  
  Serial.begin(9600); // serial comm to raspi
  //Serial.print("Setup start");
  
  //Calibrating all actuator
  move_gun(0);
  move_cam(0);
  move_tilt(0);
  
}
void loop() {
  if (Serial.available()>0)
  {
    if(get_setPoint())
    {
      move_all();   // move all actuator  
    }
  }
}

bool get_setPoint(){  //get setpoint from serial data from server
    int idx[4]; 
    char c = Serial.read();  //gets one byte from serial buffer
    String s[4];  //array of substring
    String temp;
    if (c == '\n') //while not end of serial data
    { 
      //do stuff      
      Serial.println();
      Serial.print("captured String is : "); 
      Serial.println(readStr); //prints string to serial port out
   
      idx[0] = readStr.indexOf(',');  //finds location of first ,
      s[0] = readStr.substring(0, idx[0]);   //captures first data String
      idx[1] = readStr.indexOf(',', idx[0]+1 );   //finds location of second ,
      s[1] = readStr.substring(idx[0]+1, idx[1]);   //captures second data String
      idx[2] = readStr.indexOf(',', idx[1]+1 );
      s[2] = readStr.substring(idx[1]+1, idx[2]);
      s[3] = readStr.substring(idx[2]+1); //captures remain part of data after last ,

      for (int i=0; i<4;i++)
      {
        Serial.print(" first char = ");
            Serial.print(s[i]);
        switch(s[i][0])
        {
          case 'c' :
            temp = s[i].substring(1);
             cam_set = (float) temp.toFloat();
            Serial.print(" cam set = ");
            Serial.println(cam_set);
            break;
          case 'g' :
            temp = s[i].substring(1);
             gun_set = (float) temp.toFloat();
            Serial.print(" gun set = ");
            Serial.println(gun_set);
            break;
          case 't' :
            temp = s[i].substring(1);
             tilt_set = (float) temp.toFloat();
             Serial.print(" tilt set = ");
            Serial.println(tilt_set);
            break;
          case 'y' :
            temp = s[i].substring(1);
             yaw_set = (float) temp.toFloat();
             Serial.print(" yaw set = ");
            Serial.println(yaw_set);
            break;
          default: break;
        }
      }
      readStr=""; //clears variable for new input
      Serial.flush();
      return true;
    }
    else
    {       
      readStr += c; //makes the string readString
      return false;
    }
}

void move_cam(float degree){
  float err;
  const float tolerance = 3.0;
  const float Ki_cam= 0.002, Kp_cam=1.2, Kd_cam=16;

  float cam_setd = -degree+ 90;  //map to servo degree between 0-180//reverse, EDIT if not reversed
 // servo_cam.write(cam_setd);
  cam_prev = degree;
}

void move_tilt(float degree){
  float tilt_setd = degree+ 90;  //map to servo degree between 0-180
  // set limit for pitch
  if (tilt_setd < 70) //degree < -20
  {
    tilt_setd = 70;
  }else if (tilt_setd > 150)    //degree > 60
  {
    tilt_setd = 150;
  }
  
  servo_tilt.write(tilt_setd);
  tilt_prev = degree;
}

void stepper(){
  switch(cam_stepState){
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
  if(step_dir==HIGH){ cam_stepState++;}
  if(step_dir==LOW){ cam_stepState--; }
  if(cam_stepState>7){cam_stepState=0;}
  if(cam_stepState<0){cam_stepState=7; }
}

void move_stepper(float degreeSet){
  //Serial.println("move stepper");
  float err;
  float dump;
  int step_req;
 
  err = degreeSet - cam_prev;
      
    //CW is step_dir LOW
    if (err < 0)
    {
      step_dir=HIGH;
      err = -err;
    }else
    {
      step_dir=LOW;
    }
    dump = err/step_RES;
    step_req = (int) dump;
  
    for(int x = 0; x <= step_req; x++) {  //give pulse until degree achieved
      //currentMillis = micros();
      delay(1);
      //frequensy of pulse to move stepper. Config based on datasheet
      //if(currentMillis-last_time>=1000){
      stepper();  
    }
    cam_prev=degreeSet;
}


void move_turret(float degree){
  //Function to move turret with nema 17 stepper
  float err = normalDeg(degree - yaw_act);
  int step_count;
  
  if (err>0)
  {
    digitalWrite(dirPin,LOW); // Enables the motor to move with vector direction upward the axis. In this case, move gear ccw -> move turret in clockwise direction
    step_count = err / yaw_RES;
  }else
  {
    digitalWrite(dirPin,HIGH); // Enables the motor to move turret in counterclockwise direction
    step_count = -err / yaw_RES;
  }
  /*DEBUG PURPOSE
  Serial.print("  step_count = ");
  Serial.print(step_count);
  */
  
  for (int i=0; i < step_count; i++)
  {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(500); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(500);
  }
  
  if (err>0)
  {
    yaw_act = normalDeg(yaw_act + step_count * yaw_RES);  
  }else
  {
    yaw_act = normalDeg(yaw_act - step_count * yaw_RES);
  }
  /*DEBUG PURPOSE
  Serial.print("yaw_act = ");
  Serial.print(yaw_act);
  */
}
void move_gun(float degree){
  float gun_setd = -degree;  //map to servo degree between 0-180. Reverse, EDIT this line if the servo is not reversed
  // set limit for pitch
  if (gun_setd < 70) //degree < -20
  {
    gun_setd = 70;
  }else if (gun_setd > 150)    //degree > 60
  {
    gun_setd = 150;
  }
  
  servo_gun.write(gun_setd+90);
  gun_prev = degree;
}

void move_all(){
  //DEBUG PURPOSE Serial.print("move all");
  if(cam_set != cam_prev) //if the value change
  {
    move_cam(normalDeg(cam_set-yaw_act));
  }
  if(tilt_set != tilt_prev) //if the value change
  {
    move_tilt(tilt_set);
  }
  if(yaw_set != yaw_act) //if the value change
  {
    move_turret(yaw_set);
  }
  if(gun_set != gun_prev) //if the value change
  {
    move_gun(gun_set);
  }
}


float normalDeg(float deg)
{
  //normalized degree value into -179.99 until 180
  while (deg <= -180)
  {
    deg += 360;
  }
  
  while (deg > 180)
  {
    deg -= 360;
  }
  return deg;
}

bool isTolerant(float ex, float real, float tol)
{
  // check whether the value is under accuracy of measurement tool
  bool ret = false;
  if ( ((real - ex) < tol) && ((ex-real) < tol) )
  {
    ret = true;
  }
  return ret;
}
