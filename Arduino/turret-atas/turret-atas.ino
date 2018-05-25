/*     Turret Control
 *      Created by: Naufalino Fadel Hutomo
 */
//Library
#include <Servo.h>
#include <string.h>
#include <Wire.h>
#include <avr/wdt.h>  //library watchdog timer

#define wdt_reset() __asm__ __volatile__ ("wdr")
#define servoTILT 5 //digital pin for camera tilt setpoint signal
#define servoGUN 6 //servo for control gun
#define stepPin 3 //pulse pin for controlliong turret stepper
#define dirPin 4 //pulse pin for controlliong turret stepper
#define stepPin1 8 //pulse pin for controlling stepper, stepPin1. Red
#define stepPin2 9 //pulse pin for controlling stepper, IN2. Blue
#define stepPin3 10 //pulse pin for controlling stepper, IN3. Green
#define stepPin4 11 //pulse pin for controlling stepper, IN4. Black
//#define compassPin A3// pin to control which compass is active, low for compass_cam, high for compass_turret

//Function Definition
void get_setpoint();
void move_cams(float degree);
void move_turret(float degree);
void move_tilt(float degree);
void move_stepper(float degree);
void move_gun(float degree);
void move_all();

//Servo servo_cam;  // servo object for camera yaw
Servo servo_tilt;  // servo object for camera yaw
Servo servo_gun; // // servo object for gun pitch


//CONSTANT
const float step_RES = 0.088 ; // step resolution is 360/64 step (from motor) / 64 (gearbox). If you want to change the resolution, change the driver configuration. Driver configuration 010 -> quarter step
const float yaw_RES = 0.45; //1.45 = 1.8/4 //nema 17 resolution is 1.8 degree, with gear ratio between motor and turret is 1:4

String readStr;
float cam_set = 0;  //for camera pan set condition
float cam_prev = 0 , cam_prev2=0;
float cam_act=0, cam_act_init=0;  //heading measurement of camera
float cam_sum=0, cam_last_pid=0; // for PID calculation of camera movement
int cam_stepState;

float tilt_set = 0;  //for camera tilt set condition
float tilt_prev = 0 , tilt_prev2=0;
float yaw_set = 0;   //for turret yaw set condition
float yaw_act=0,yaw_act_prev=0, yaw_act_prev2=0;  //heading measurement of yaw
float yaw_prev = 0;

float gun_set = 0;   //for gun pitch set condition
float gun_prev = 0 , gun_prev2=0;

bool step_dir = HIGH; //CW is HIGH


void setup() {
  //WD/  
  wdt_disable();
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
  
  Serial.begin(115200); // serial comm to raspi
  Serial.print("Setup start");
  
  //Calibrating all actuator
  move_gun(0);
  move_cams(0);
  move_tilt(0);

  //WD/    
  wdt_enable(WDTO_8S); //enable watchdog timer, timeout value 8s
  
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

void watchdog_setup(int t){
  //This function set the register for watchdog timer
  
  //WDP3 - WDP2 - WPD1 - WDP0 - time
  // 0      0      0      0      16 ms
  // 0      0      0      1      32 ms
  // 0      0      1      0      64 ms
  // 0      0      1      1      0.125 s
  // 0      1      0      0      0.25 s
  // 0      1      0      1      0.5 s
  // 0      1      1      0      1.0 s
  // 0      1      1      1      2.0 s
  // 1      0      0      0      4.0 s
  // 1      0      0      1      8.0 s


  // Reset the watchdog reset flag
  bitClear(MCUSR, WDRF);
  // Start timed sequence
  bitSet(WDTCSR, WDCE); //Watchdog Change Enable to clear WD
  bitSet(WDTCSR, WDE); //Enable WD

  switch (t){
    case 8 : 
    // Set new watchdog timeout value to 8 second
    bitSet(WDTCSR, WDP3);
    bitSet(WDTCSR, WDP0);  
    break;
    default:    //default is 2s
    bitSet(WDTCSR, WDP2);
    bitSet(WDTCSR, WDP1);
    bitSet(WDTCSR, WDP0);
  }
   
  // Enable interrupts instead of reset
  //bitSet(WDTCSR, WDIE);
}

ISR(WDT_vect) {
  // Don't do anything here but we must include this
  // block of code otherwise the interrupt calls an
  // uninitialized interrupt handler.
}

bool get_setPoint(){  //get setpoint from serial data from server
    int idx[4]; 
    char c = Serial.read();  //gets one byte from serial buffer
    String s[4];  //array of substring
    String temp;
    if (c == '\n') //while not end of serial data
    { 
      //do stuff      
      ///DEBUGSerial.println();
      ///DEBUGSerial.print("captured String is : "); 
      ///DEBUGSerial.println(readStr); //prints string to serial port out
   
      idx[0] = readStr.indexOf(',');  //finds location of first ,
      s[0] = readStr.substring(0, idx[0]);   //captures first data String
      idx[1] = readStr.indexOf(',', idx[0]+1 );   //finds location of second ,
      s[1] = readStr.substring(idx[0]+1, idx[1]);   //captures second data String
      idx[2] = readStr.indexOf(',', idx[1]+1 );
      s[2] = readStr.substring(idx[1]+1, idx[2]);
      s[3] = readStr.substring(idx[2]+1); //captures remain part of data after last ,

      for (int i=0; i<4;i++)
      {
        ///DEBUGSerial.print(" first char = ");
        ///DEBUG    Serial.print(s[i]);
        switch(s[i][0])
        {
          case 'c' :
            temp = s[i].substring(1);
             cam_set = (float) temp.toFloat();
            ///DEBUGSerial.print(" cam set = ");
            ///DEBUGSerial.println(cam_set);
            break;
          case 'g' :
            temp = s[i].substring(1);
             gun_set = (float) temp.toFloat();
            ///DEBUGSerial.print(" gun set = ");
            ///DEBUGSerial.println(gun_set);
            break;
          case 't' :
            temp = s[i].substring(1);
             tilt_set = (float) temp.toFloat();
             ///DEBUGSerial.print(" tilt set = ");
            ///DEBUGSerial.println(tilt_set);
            break;
          case 'y' :
            temp = s[i].substring(1);
             yaw_set = (float) temp.toFloat();  //this data came from nodeMCU that control stepper
             ///DEBUGSerial.print(" yaw act = ");
            ///DEBUGSerial.println(yaw_act);
            break;
          default: break;
        }
      }
      
      readStr=""; //clears variable for new input
      Serial.flush();
      //WD      
      wdt_reset();  //reset the watch dog timer 
      return true;
    }
    else
    {       
      readStr += c; //makes the string readString
      return false;
    }
}

/*
void move_cam(float degree){
  float err;
  const float tolerance = 3.0;
  const float Ki_cam= 0.002, Kp_cam=1.2, Kd_cam=16;

  float cam_setd = -degree+ 90;  //map to servo degree between 0-180//reverse, EDIT if not reversed
 // servo_cam.write(cam_setd);
  updatePrev('c');
}*/

void move_tilt(float degree){
  float tilt_setd = degree+ 90;  //map to servo degree between 0-180. //direction of servo is reversed because of current servo configuration.
  // set limit for pitch
  if (tilt_setd < 70) //degree < -20
  {
    tilt_setd = 70;
  }else if (tilt_setd > 150)    //degree > 60
  {
    tilt_setd = 150;
  }
  
  servo_tilt.write(tilt_setd); 
  updatePrev('t');
  //DEBUG
  //Serial.print("\n tilt ");Serial.print(tilt_prev2);Serial.print(tilt_prev);Serial.print(tilt_set);
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

void move_cams(float degreeSet){
  //Serial.println("move stepper");
  float err;
  float dump;
  int step_req;
 
  err = normalDeg(degreeSet - cam_act);
      
    //CW is step_dir LOW
    if (err < 0)
    {
      step_dir=HIGH;
      step_req = round(-err/step_RES);
    }else
    {
      step_dir=LOW;
      step_req = round(err/step_RES);
    }
  
    for(int x = 1; x <= step_req; x++) {  //give pulse until degree achieved
      //currentMillis = micros();
      delay(2);
      //frequensy of pulse to move stepper. Config based on datasheet
      //if(currentMillis-last_time>=1000){
      stepper();  
    }

    if (err>0)  //calculate camera direction after movement
    {
      cam_act = normalDeg(cam_act + step_req * step_RES);    
    }else
    {
      cam_act = normalDeg(cam_act - step_req * step_RES);    
    }
    
    updatePrev('c');
   ///DEBUG
   Serial.print("cam_act = ");
   ///DEBUG  
   Serial.print(cam_act);
   ///DEBUG   
   Serial.print(" \tcam_set = ");
   ///DEBUG   
   Serial.println(cam_set);
   ///DEBUG   Serial.print(" \tstep = ");
   ///DEBUG   Serial.println(step_req);

    ///DEBUGSerial.print("\n cam ");Serial.print(cam_prev2);Serial.print(cam_prev);Serial.print(cam_act);

}


void move_turret(float degree){
  //Function to move turret with nema 23 stepper
  float err = normalDeg(degree - yaw_act);
  int step_count;
  
  if (err>=0)
  {
    //digitalWrite(dirPin,LOW); // Enables the motor to move with vector direction upward the axis. In this case, move gear ccw -> move turret in clockwise direction
    step_count = round(err / yaw_RES);
  }else
  {
    //digitalWrite(dirPin,HIGH); // Enables the motor to move turret in counterclockwise direction
    step_count = round(-err / yaw_RES);
  }
  /*DEBUG PURPOSE
  Serial.print("  step_count = ");
  Serial.print(step_count);
  */
  
  //give pulse to stepper
  /* COMMENT THIS LINE BC THE TURRET IS MOVED SEPARATELY BY NODEMCU
  for (int i=1; i < step_count; i++)
  {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(500); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(500);
  } */
  
  if (err>0)
  {
    yaw_act = normalDeg(yaw_act + step_count * yaw_RES);
    cam_act = normalDeg(cam_act + step_count * yaw_RES);    //to compensate camera direction due to turret movement
  }else
  {
    yaw_act = normalDeg(yaw_act - step_count * yaw_RES);
    cam_act = normalDeg(cam_act - step_count * yaw_RES);    //to compensate camera direction due to turret movement
  }

  //move camera to initial setpoint before turret moved
  //move_cams(cam_set);

  ///DEBUG   Serial.print("move turret \t");
  ///DEBUG   Serial.print("yaw act = ");
  ///DEBUG   Serial.print(yaw_act);
  ///DEBUG   Serial.print(" \tyaw_set = ");
  ///DEBUG   Serial.print(yaw_set);
  ///DEBUG   Serial.print(" \tcam_act = ");
  ///DEBUG   Serial.print(cam_act);
  /*DEBUG PURPOSE
  Serial.print("yaw_act = ");
  Serial.print(yaw_act);
  */
}

void move_gun(float degree){
  float gun_setd = degree;  //map to servo degree between 0-180. Reverse, EDIT this line if the servo is not reversed
  // set limit for pitch
  gun_setd *= 4.5;    //4.5 is the ratio between servo degree and gun degree. The factor comes from experiment. 
  gun_setd += 90;     // Map to servo degree between 0-180
  if (gun_setd < 20) //degree < -20
  {
    gun_setd = 20;
  }else if (gun_setd > 170)    //degree > 60
  {
    gun_setd = 170;
  }
  
  servo_gun.write(gun_setd);
  updatePrev('g');
    //DEBUGSerial.print("\n gun ");Serial.print(gun_prev2);Serial.print(gun_prev);Serial.print(gun_set);

}

void move_all(){
  
  if(tilt_set != tilt_prev) //if the value change
  {
    ///DEBUGtilt_set = filter('t');
    move_tilt(tilt_set);
  }
  /**/
  if(yaw_set != yaw_act) //if the value change
  {
    move_turret(yaw_set);
  }
  if(cam_set != cam_act) //if the value change
  {
    ///DEBUGcam_set = filter('c');
    move_cams(cam_set);
  }
  if(gun_set != gun_prev) //if the value change
  {
    ///DEBUGgun_set = filter('g');
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


float filter (char c)
{
  //RECONSIDER ABOUT THE ALGORITHM
  // filter the serial data come to Arduino
  float prev, prev2, now;  // data that need to be filtered
  float grad1, grad2;
  
  switch (c){
    case 'c':
      now= cam_set;
      prev = cam_prev;
      prev2 = cam_prev2;
      break;
    case 't':
      now= tilt_set;
      prev = tilt_prev;
      prev2 = tilt_prev2;
      break;
    case 'y':
      now= yaw_act;
      prev = yaw_act_prev;
      prev2 = yaw_act_prev2;
      break;
    case 'g':
      now= gun_set;
      prev = gun_prev;
      prev2 = gun_prev2;
      break;
    default:
      break;
  }

  //check the noise because of parsing error that cause shifting
  if ((now > 200) or (now<-200))  // data out of range. Degree are between 0-180 and -179-0
  {
    return prev;
    
  } 

  //check the noise because of parsing error
  grad1 = (now-prev);
  grad2 = (prev-prev2);
  if ( ( (now >= (9*prev)) && (now > 10+prev) ) or ((now <= (prev/9) ) && (now < prev-10) ) ) // if the gradient is too high
  {
    //check whether the signal is a peak noise or a consistent increasing/decreasing signal
      return prev; 
  }else {
    return now;
  }
  
}


float updatePrev (char c)
{
  // update the previous value after move the actuator
  
  switch (c){
    case 'c':
      cam_prev2 = cam_prev;
      cam_prev = cam_act;
      break;
    case 't':
      tilt_prev2 = tilt_prev;
      tilt_prev = tilt_set;
      break;
    case 'y':
      yaw_act_prev2 = yaw_act_prev;
      yaw_act_prev = yaw_act;
      break;
    case 'g':
      gun_prev2 = gun_prev;
      gun_prev = gun_set;
      break;
    default:
      break;
  }  
}
