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
  Serial.print("Setup start");
  
  //Calibrating all actuator
  move_gun(0);
  move_cam(0);
  move_tilt(0);
  
}
void loop() {
  if (Serial.available()>0)
  {
    get_setpoint();
    move_all();   // move all actuator
  }
}


void get_setpoint(){   //get setpoint from Server, through serial comms
  String serialResponse;
  serialResponse = Serial.readStringUntil('\n');
  Serial.flush();
  Serial.print(serialResponse);
  String bufS = serialResponse;
  char buf[sizeof(serialResponse)];
  serialResponse.toCharArray(buf, sizeof(buf));
  char  *p = buf;
  char *camc, *gunc, *tiltc, *yawc;  //string variable for cam and gun
  
  //char *str;
  //str = strtok_r(p, ",", &p);
  String str;
  str = buf.substring(0,bufS.indexOf(","));
  Serial.println(str); 
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
      Serial.println(str);
    } else if(str[0]=='t')        // get camera tilt setpoint
    {
      tiltc = &str[1];
        String s=tiltc;
      tilt_set = (float) s.toFloat();
      Serial.print("tilt_set = ");
      Serial.println(tilt_set);
      str = strtok_r(p, ",", &p);
      Serial.println(str);
    } else if(str[0]=='y')      //get turret yaw setpoint
    {
      yawc = &str[1];
      String s=yawc;
      yaw_set = (float) s.toFloat();
      Serial.print("yaw_set = ");
      Serial.println(yaw_set);
      str = strtok_r(p, ",", &p);
      Serial.println(str);
    }else if(str[0]=='g')         // get gun setpoint
    {
      gunc = &str[1];
      String s=gunc;
      gun_set = (float) s.toFloat();
      str = strtok_r(p, ",", &p);
      Serial.print("gun_set = ");
      Serial.println(gun_set);
      Serial.println(str);
    }
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
  Serial.println("move stepper");
  float err;
  float dump;
  int step_req;
 
  err = degreeSet - cam_prev;
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
  
  Serial.print("  step_count = ");
  Serial.print(step_count);
  
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
  Serial.print("yaw_act = ");
  Serial.print(yaw_act);
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
    move_cam(normalDeg(cam_set-yaw_act));
  }
  if(tilt_set != tilt_prev) //if the value change
  {
    move_tilt(tilt_set);
  }
  if(yaw_set != yaw_act) //if the value change
  {
    move_turret(yaw_set);
  } else if(gun_set != gun_prev) //if the value change
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
