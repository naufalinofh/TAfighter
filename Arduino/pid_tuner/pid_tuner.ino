/*     Turret Control
 *      Created by: Naufalino Fadel Hutomo
 *      
 */
//Library
#include <Servo.h>
#include <Wire.h>
#include "QMC5883_run.h"  //For Compass Sensor QMC5883L

#define servoCAM 3 //digital pin for camera pan setpoint signal


//Function Definition
void move_cam(double degree);


Servo servo_cam;  // servo object for camera yaw


QMC5883 compass;

//CONSTANT
const float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);   // You can find your declination on: http://magnetic-declination.com/... For Bandungdeclination angle is 4'26E (positive)

float cam_set = 0;  //for camera pan set condition
float cam_prev = 0;
float cam_act=0, cam_act_init=0;  //heading measurement of camera
float cam_sum=0, cam_last_pid=0; // for PID calculation of camera movement
float setpoint = 90.0;

float getHeading();
void setup() {
  // Sets pins Mode

  servo_cam.attach(servoCAM);
  servo_cam.write(0);//make the servo steady
  Serial.begin(9600); // serial comm to raspi
  //COMPASS SETUP 
  while (!compass.begin())
  {
    Serial.println("Could not find a valid QMC5883 sensor, check wiring!");
    delay(500);
  }
  Serial.println("wait 2 seconds");
  delay(2000);
  Serial.println("Calibrating compass, move the compass");
  
  for (int i =0; i<200; i++)
  {
    if (i%100 ==0){
      Serial.print(i/100);
    Serial.print(" .. ");  
    }
    
    cam_act = getHeading();
    delay(10);
  }
  Serial.println("Calibrating initial compass heading, be steady");
  delay(5000);
  cam_act_init = getHeading();
  delay(2000);
  
  // END of COMPASS SETUP
}
void loop() {
  
  Serial.print("setpoint");
    Serial.print(",");
    Serial.print("cam_act");
    Serial.print(",");
    Serial.println("cam_setd");
    delay(2000);
  /*
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
      cam_set = (float) s.toFloat();
      Serial.print("cam_set = ");
      Serial.println(cam_set);
      str = strtok_r(p, ",", &p);
    }
  }
  servo_cam.write(cam_set);
  */
  move_cam(setpoint);
  delay(5000);
  //setpoint = 0;
  //move_cam(setpoint);
  //delay(5000);
  setpoint=90;

  
}


void move_cam(float degree){
  //float cam_setd = degree+ 90;  //map to servo degree between 0-180
  //servo_cam.write(cam_setd);
  float err;
  const float tolerance = 0.5;
  const float Ki_cam= 0.0023, Kp_cam=1.2088, Kd_cam=161;

  cam_act = normalDeg(getHeading()-cam_act_init);
  err = degree - cam_act; // find error between setpoint and actual measurement
  while(! (isTolerant(0,err,tolerance))) //if the error is not under tolerance of system
  {
    const float cam_max = 180;
    //PID calculation
    cam_sum +=err;  //cumulative error for integrative controller 
    if(cam_sum > cam_max) cam_sum= cam_max;
      else if(cam_sum < cam_max*-1) cam_sum= cam_max*-1;

    float cam_pid_out = Kp_cam * err + Ki_cam * (cam_sum) + Kd_cam * (err - cam_last_pid);
    
    if(cam_pid_out > cam_max) cam_pid_out= cam_max;
      else if(cam_pid_out < cam_max*-1) cam_pid_out= cam_max*-1;

    cam_last_pid=err; // renew the last value
    //end of PID calculation

    //float cam_setd = map(cam_pid_out,-tolerance*2.5,tolerance*2.5, 83,97);
    float cam_setd = servo_control(cam_pid_out);
    servo_cam.write(cam_setd);//move the servo
    //Serial.print("PID output ="); //DEBUG 
    //Serial.println(cam_setd); 
    
    Serial.print(millis());
    Serial.print(",");
    Serial.print(setpoint);
    Serial.print(",");
    Serial.print(cam_act);
    Serial.print(",");
    Serial.println(cam_setd);
    
    delay(1);
    cam_act = normalDeg(getHeading()-cam_act_init); //measure again
    err = degree - cam_act;
  }
  cam_prev = degree;
}

float getHeading()
{
  const float sampling_qty = 10.0;
  Vector norm;
  float heading;
  float y=0, x=0;
  int i=0;
  unsigned long last_time = micros(),currentMillis = micros();

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


float servo_control(float out)
{
  float ret =0;
  
  if (out < 0.5 && out > -0.5)
  {
      ret = 0;
  }else if(out > 20.5)
  {
    ret = 81.5;
  }else if(out < -20.5)
  {
    ret = 99.5;
  }else if (out > 2.5)
  {
    ret = -0.1429* out + 84.357;
  }else if(out < -2.5)
  {
    ret = -0.1429* out + 96.643;
  }else if (out > 0.5)
  {
    ret = -0.5* out + 85.25;
  }else if(out < -0.5)
  {
    ret = -0.5* out + 95.75;
  }

  return ret;
}

