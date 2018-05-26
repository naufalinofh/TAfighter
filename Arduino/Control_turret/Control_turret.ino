/*     Turret Control
 *      Created by: Naufalino Fadel Hutomo
 *      
 */
//Library
#include <Servo.h>
#include <string.h>
#include <Wire.h>
#include "QMC5883_run.h"  //For Compass Sensor QMC5883L

//#include <Wire.h>
//#include "I2Cdev.h" //For MPU6050 
//#include "MPU6050.h" //For MPU6050 

//MPU6050 mpuc; //For MPU6050 

#define servoCAM 3 //digital pin for camera pan setpoint signal
#define servoTILT 5 //digital pin for camera tilt setpoint signal
#define servoGUN 6 //servo for control gun
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

Servo servo_cam;  // servo object for camera yaw
Servo servo_tilt;  // servo object for camera yaw
Servo servo_gun; // // servo object for gun pitch

QMC5883 compass;

//CONSTANT
const float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);   // You can find your declination on: http://magnetic-declination.com/... For Bandungdeclination angle is 4'26E (positive)
const float step_RES = 0.044 ; // step resolution is 360/64 step (from motor) / 64 (gearbox)/2 (gear turret). If you want to change the resolution, change the driver configuration. Driver configuration 010 -> quarter step

//int16_t gx, gy, gz;  // gyro value
float cam_set = 0;  //for camera pan set condition
float cam_prev = 0;
float cam_act=0, cam_act_init=0;  //heading measurement of camera
float cam_sum=0, cam_last_pid=0; // for PID calculation of camera movement

float tilt_set = 0;  //for camera tilt set condition
float tilt_prev = 0;
float yaw_set = 0;   //for turret yaw set condition
float yaw_prev = 0;
float yaw_act=0, yaw_act_init=0;  //heading measurement of yaw
float gun_set = 0;   //for gun pitch set condition
float gun_prev = 0;
bool step_dir = HIGH; //CW is HIGH
bool compassSync = false; //check whether the compass is connect
bool useCompass = false;
int step_count=0;// number of steps
//unsigned long currentMillis=0, last_time=0;

void setup() {
  // Sets pins Mode
  //pinMode(stepPin,OUTPUT); 
  pinMode(stepPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(stepPin3, OUTPUT);
  pinMode(stepPin4, OUTPUT);
  servo_cam.attach(servoCAM);
  servo_tilt.attach(servoTILT);
  servo_gun.attach(servoGUN);
  pinMode(compassPin, OUTPUT);
  digitalWrite(compassPin, LOW);
  
  Serial.begin(9600); // serial comm to raspi
  Serial.print("Setup start");
  if (useCompass)
  {
    //COMPASS SETUP 
    int i = 0;
    compassSync = true;
    while ( (compassSync ==false) || i <5 )
    {
      Serial.println("Could not find a valid QMC5883 sensor, check wiring!");
      //compassSync = //compass.begin();
      i++;
      delay(500);
    }
    
    Serial.println("Calibrating compass, move the compass");
    for (int i =0; i<2; i++)
    {
      Serial.print(i);
      Serial.print(" .. ");
      cam_act = getHeading('c');
    }
    Serial.println("Calibrating initial compass heading, be steady");
    delay(200);
    cam_act_init = getHeading('c');
    yaw_act_init = getHeading('y');  
  }
  
  delay(200);
  //Calibrating all actuator
  move_gun(0);
  move_cam(0);
  move_tilt(0);
  
  // END of COMPASS SETUP

  //Serial.println("Initialize MPU");
  //mpuc.initialize();
  //Serial.println(mpuc.testConnection() ? "Connected" : "Connection failed");
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
  char  *p = buf;
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
      cam_set = (float) s.toFloat();
      Serial.print("cam_set = ");
      Serial.println(cam_set);
      str = strtok_r(p, ",", &p);
    } else if(str[0]=='t')        // get camera tilt setpoint
    {
      tiltc = &str[1];
      String s=tiltc;
      tilt_set = (float) s.toFloat();
      Serial.print("tilt_set = ");
      Serial.println(tilt_set);
      str = strtok_r(p, ",", &p);
    } else if(str[0]=='y')      //get turret yaw setpoint
    {
      yawc = &str[1];
      String s=yawc;
      yaw_set = (float) s.toFloat();
      Serial.print("yaw_set = ");
      Serial.print(yaw_set);
      str = strtok_r(p, ",", &p);
    }else if(str[0]=='g')         // get gun setpoint
    {
      gunc = &str[1];
      String s=gunc;
      gun_set = (float) s.toFloat();
      str = strtok_r(p, ",", &p);
      Serial.print("gun_set = ");
      Serial.println(gun_set);
    }
  }
}

void move_cam(float degree){
  float err;
  const float tolerance = 3.0;
  const float Ki_cam= 0.002, Kp_cam=1.2, Kd_cam=16;

  if (useCompass)
  {
    cam_act = normalDeg(getHeading('c')-cam_act_init);
    err = degree - cam_act; // find error between setpoint and actual measurement
    while(!isTolerant(0,err,tolerance)) //if the error is not under tolerance of system
    {
      const float cam_max = 180;
      //PID calculation
      cam_sum +=err;  //cumulative error for integrative controller 
      if(cam_sum > cam_max) cam_sum= cam_max;
        else if(cam_sum < cam_max*-1) cam_sum= cam_max*-1;
  
      float cam_pid_out = Kp_cam * err + Ki_cam * (cam_sum) + Kd_cam * (err - cam_last_pid);
      
      if(cam_pid_out > cam_max) {cam_pid_out= cam_max;}
        else if(cam_pid_out < cam_max*-1) cam_pid_out= cam_max*-1;
  
      cam_last_pid=err; // renew the last value
      //end of PID calculation
  
      //servo control eq. Input degree, Output PWM 
      float cam_setd = map(cam_pid_out,-tolerance*2.5,tolerance*2.5,-15,15);
      servo_cam.write(cam_setd);//move the servo
      Serial.print("PID output ="); //DEBUG 
      Serial.println(cam_setd);
      cam_act = normalDeg(getHeading('c')-cam_act_init); //measure again
      err = degree - cam_act; 
    }
  }else {//not use compass
    float cam_setd = -degree+ 90;  //map to servo degree between 0-180//reverse, EDIT if not reversed
    servo_cam.write(cam_setd);
  }
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

void move_stepper(float degreeSet){
  Serial.println("move stepper");
  float err;
  float dump;
  int step_req;
  Serial.print("  yaw set = ");
  Serial.print(yaw_set);
  if (useCompass)
  {
    yaw_act = normalDeg(getHeading('y')-yaw_act_init);
    float deg = normalDeg(degreeSet-yaw_act);
    err = deg;
   
    //Serial.print("degree =");
    //Serial.print(deg);
    const float tolerance = 3.0;
    while (!isTolerant(0,err,tolerance)) //if the error is not under tolerance of system
    {   
      //CW is step_dir LOW
      if (err < 0)
      {
        step_dir=HIGH;
        err = -err;
      }else
      {
        step_dir=LOW;
      }
  
      dump = deg/step_RES;
      step_req = (int) dump;
  
      //while (steps_left>0){
      for(int x = 0; x <= step_req; x++) {  //give pulse until degree achieved
        //currentMillis = micros();
        delay(1);
        //frequensy of pulse to move stepper. Config based on datasheet
        //if(currentMillis-last_time>=1000){
        stepper(); 
          //last_time=micros();
          //steps_left--;
         //} 
      }
      yaw_act = normalDeg(getHeading('y')-yaw_act_init);
      err = normalDeg(degreeSet-yaw_act);
       
       
      //Serial.print(step_req);
    } // end of while. Is under tolerance
  
    //yaw_set +=degree;
    yaw_prev = yaw_act;  
  } else    //not use compass
  {
    err = degreeSet - yaw_prev;
      Serial.print("  err = ");
  Serial.print(err);    
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
    yaw_act = degreeSet;
    yaw_prev=yaw_act;
  }
  
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
    move_stepper(yaw_set);
  } else if(gun_set != gun_prev) //if the value change
  {
    move_gun(gun_set);
  }
}

float getHeading(char c)
{
  Vector norm;
  float heading;
  float y=0, x=0;
  int i=0;
  unsigned long last_time = micros(),currentMillis = micros();

  //Check which compass we would to get data on
  switch (c){
    case 'c':
        digitalWrite(compassPin, LOW);
        break;
    case 'y':
        digitalWrite(compassPin, HIGH);
        break;
    default:
        digitalWrite(compassPin, LOW);
        break;
  }
   
  while ( (currentMillis=micros())-last_time < 100)
  {
    norm = compass.readNormalize();
    y += norm.YAxis;
    x += norm.XAxis;
    i++;
  } //end of sampling
  
  if(currentMillis-last_time>=100){
    last_time=micros();
    y = y/i;  //averaging
    x = x/i;
  } 
  
  // Calculate heading
  heading = atan2(y, x);
  heading += declinationAngle;   

  // Correct for heading < 0deg and heading > 360deg
  if (heading < -PI){
    heading += 2 * PI;
  }

  if (heading > PI){
    heading -= 2 * PI;
  }

  // Convert to degrees
  heading = normalDeg( heading * 180/PI); 

  delay(10);
  return heading;
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

/*
float pid_calc(char var)
{
  switch (var){
    case 'c' :
      float max = 15;
      float min = -15;
      float Kp= 1;
      float Ki= 1;
      float Kd= 0.2;
      cam_sum += Ki * err;
      if(cam_sum > max) outputSum= outMax;
      else if(outputSum < outMin) outputSum= outMin;
      break;
    case 'y' :
      
      break;
  }
}
*/
