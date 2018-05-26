/*     Turret Control
 *      Created by: Naufalino Fadel Hutomo
 *      
 */
//Library
#include <string.h>
#include <Wire.h>

#define stepPin 3 //pulse pin for controlliong turret stepper
#define dirPin 4 //pulse pin for controlliong turret stepper

//CONSTANT
const float yaw_RES = 1.8/4 ; //nema 17 resolution is 1.8 degree, with gear ratio between motor and turret is 1:4

String readStr;
float yaw_set = 0;   //for turret yaw set condition
float yaw_act=0;  //heading measurement of yaw

bool step_dir = HIGH; //CW is HIGH


void setup() {
  // Sets pins Mode
  //pinMode(stepPin,OUTPUT); 
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  
  Serial.begin(9600); // serial comm to raspi
  Serial.print("Setup start");
}

void loop() {
  if (Serial.available()>0)
  {
    if(get_setPoint())  //the setpoint has all set
    {
      move_all();   // move all actuator  
    }   
  }
}

bool get_setPoint(){
    int idx[4]; 
    char c = Serial.read();  //gets one byte from serial buffer
    String s[4];  //array of substring
    String temp;
    if (c == '\n') //while not end of serial data
    { 
      //do stuff      
      /* DEBUG PURPOSE
      Serial.println();
      Serial.print("captured String is : "); 
      Serial.println(readStr); //prints string to serial port out
      */
      
      idx[0] = readStr.indexOf(',');  //finds location of first ,
      s[0] = readStr.substring(0, idx[0]);   //captures first data String
      idx[1] = readStr.indexOf(',', idx[0]+1 );   //finds location of second ,
      s[1] = readStr.substring(idx[0]+1, idx[1]);   //captures second data String
      idx[2] = readStr.indexOf(',', idx[1]+1 );
      s[2] = readStr.substring(idx[1]+1, idx[2]);
      s[3] = readStr.substring(idx[2]+1); //captures remain part of data after last ,

      for (int i=0; i<4;i++)
      {
        switch(s[i][0])
        {
          case 'y' :
            temp = s[i].substring(1);
             yaw_set = (float) temp.toFloat();
             /*DEBUG Purpose Serial.print(" yaw set = ");
            Serial.println(yaw_set);
            */
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

void move_turret(float degree){
  //Function to move turret with nema 17 stepper
  float err = normalDeg(degree - yaw_act);
  int step_count;
  
  if (err>0)
  {
    digitalWrite(dirPin,LOW); // Enables the motor to move with vector direction upward the axis. In this case, move gear ccw -> move turret in clockwise direction
    step_count = round(err / yaw_RES);
  }else
  {
    digitalWrite(dirPin,HIGH); // Enables the motor to move turret in counterclockwise direction
    step_count = round(-err / yaw_RES);
  }
  
  
  for (int i=0; i < step_count; i++)
  {
    digitalWrite(stepPin,HIGH); 
    delay(2); 
    digitalWrite(stepPin,LOW); 
    delay(2);
  }
  
  if (err>0)
  {
    yaw_act = normalDeg(yaw_act + step_count * yaw_RES);  
  }else
  {
    yaw_act = normalDeg(yaw_act - step_count * yaw_RES);
  }
  
  ///DEBUG
  Serial.print("yaw_act = ");
  ///DEBUG
  Serial.print(yaw_act);
  
}

void move_all(){
  if(yaw_set != yaw_act) //if the value change
  {
    move_turret(yaw_set);
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
