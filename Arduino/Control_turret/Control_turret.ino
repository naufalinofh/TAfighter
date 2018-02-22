/*     Turret Control
 *      Created by: Naufalino Fadel Hutomo
 *      
 */
//Library
#include <Servo.h>
#include <string.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 mpuc;

#define servoCAM 9 //digital pin for camera pan setpoint signal
#define servoTILT 5 //digital pin for camera tilt setpoint signal
#define servoGUN 6 //servo for control gun
#define stepPin1 8 //pulse pin for controlling stepper, stepPin1
#define stepPin2 9 //pulse pin for controlling stepper, IN2
#define stepPin3 10 //pulse pin for controlling stepper, IN3
#define stepPin4 11 //pulse pin for controlling stepper, IN4

//Function Definition
void get_setpoint();
void move_cam(double degree);
void move_tilt(double degree);
void move_stepper(double degree);
void move_gun(double degree);
void move_all();

Servo servo_cam;  // servo object for camera yaw
Servo servo_tilt;  // servo object for camera yaw
Servo servo_gun; // // servo object for gun pitch

const double step_RES = 0.044 ; // step resolution is 360/64 step (from motor) / 64 (gearbox)/2 (gear turret). If you want to change the resolution, change the driver configuration. Driver configuration 010 -> quarter step

int16_t gx, gy, gz; // gyro value
double cam_set = 0;   //for camera pan set condition
double cam_prev = 0;
double cam_act=0;
double tilt_set = 90;  //for camera tilt set condition
double tilt_prev = 90;
double yaw_set = 0;   //for turret yaw set condition
double yaw_prev = 0;
double yaw_act=0;  //
double gun_set = 90;   //for gun pitch set condition
double gun_prev = 90;
boolean step_dir = HIGH; //CW is HIGH
int step_count=0;// number of steps
//unsigned long currentMillis=0, last_time=0;

void setup() {
  // Sets pins Mode
  //pinMode(stepPin,OUTPUT); 
  pinMode(stepPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(stepPin3, OUTPUT);
  pinMode(stepPin4, OUTPUT);
  //pinMode(dirPin,OUTPUT);
  servo_cam.attach(servoCAM);
  servo_tilt.attach(servoTILT);
  servo_gun.attach(servoGUN);

  Serial.begin(9600); // serial comm to raspi
  Wire.begin();

  Serial.println("Initialize MPU");
  mpuc.initialize();
  Serial.println(mpuc.testConnection() ? "Connected" : "Connection failed");
  //Serial.print("Setup success");
}
void loop() {
  if (Serial.available())
  {
    get_setpoint();
    move_all();   // move all actuator
  }
}


void get_setpoint(){   //get setpoint from Server, through serial comms
  String serialResponse;
  serialResponse = Serial.readStringUntil('\0');
  
  char buf[sizeof(serialResponse)];
  serialResponse.toCharArray(buf, sizeof(buf));
  char *p = buf;
  char *camc, *gunc, *tiltc, *yawc;  //string variable for cam and gun
  
  char *str;
  str = strtok_r(p, ",", &p); 
  while (str  != NULL) // delimiter is the semicolon
  {
    //Serial.print(str);
    //Serial.println(str[0]);
    if(str[0]=='c')               //get camera pan setpoint
    {
      camc = &str[1];
      String s=camc;
      cam_set = (double) s.toFloat();
      Serial.print("cam_set = ");
      Serial.println(cam_set);
      str = strtok_r(p, ",", &p);
    } else if(str[0]=='t')        // get camera tilt setpoint
    {
      tiltc = &str[1];
      String s=tiltc;
      tilt_set = (double) s.toFloat();
      Serial.print("tilt_set = ");
      Serial.println(tilt_set);
      str = strtok_r(p, ",", &p);
    } else if(str[0]=='y')      //get turret yaw setpoint
    {
      yawc = &str[1];
      String s=yawc;
      yaw_set = (double) s.toFloat();
      Serial.print("yaw_set = ");
      Serial.print(yaw_set);
      str = strtok_r(p, ",", &p);
    }else if(str[0]=='g')         // get gun setpoint
    {
      gunc = &str[1];
      String s=gunc;
      gun_set = (double) s.toFloat();
      str = strtok_r(p, ",", &p);
      Serial.print("gun_set = ");
      Serial.println(gun_set);
    }
  }
}

void move_cam(double degree){
  double cam_setd = degree+ 90;  //map to servo degree between 0-180
  servo_cam.write(cam_setd);
  cam_prev = degree;
}

void move_tilt(double degree){
  double tilt_setd = degree+ 90;  //map to servo degree between 0-180
  servo_tilt.write(tilt_setd);
  tilt_prev = degree;
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
  
  yaw_set +=degree;
  yaw_prev = yaw_set;
  yaw_act = yaw_prev;
  delay(10);
}

void move_gun(double degree){
  double gun_setd = degree + 90;  //map to servo degree between 0-180
  servo_gun.write(gun_setd);
  gun_prev = degree;
}

void move_all(){
  Serial.print("move all");
  if(cam_set != cam_prev) //if the value change
  {
    move_cam(cam_set-yaw_act);
  }
  if(tilt_set != tilt_prev) //if the value change
  {
    move_tilt(tilt_set);
  }
  if(yaw_set != yaw_prev) //if the value change
  {
    move_stepper(yaw_set-yaw_act);
  } else if(gun_set != gun_prev) //if the value change
  {
    move_gun(gun_set);
  }
}

double getYaw (*x, *y, *z){
  mpuc.getMotion(&ax, &ay, &az, &gx, &gy, &gz);
  val = map(ay, -17000, 17000, 0, 179);
}

