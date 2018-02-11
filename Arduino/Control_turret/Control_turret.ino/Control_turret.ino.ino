/*     Turret Control
 *      Created by: Naufalino Fadel Hutomo
 *      
 */
//Library
#include <Servo.h>
#include <string.h>

#define servoCAM 9 //digital pin for camera pan setpoint signal
#define servoTILT 5 //digital pin for camera tilt setpoint signal
#define servoGUN 6 //servo for control gun
#define stepPin 3 //pulse pin for controlling stepper
#define dirPin 4 //to control stepper direction

Servo servo_cam;  // servo object for camera yaw
Servo servo_tilt;  // servo object for camera yaw
Servo servo_gun; // // servo object for gun pitch

const double step_RES = 1.4 ; // step resolution is 360/64 step (from motor) / 4 (driver). If you want to change the resolution, change the driver configuration

double cam_set = 0;   //for camera pan set condition
double cam_prev = 0;
double tilt_set = 90;  //for camera tilt set condition
double tilt_prev = 90;
double yaw_set = 0;   //for turret yaw set condition
double yaw_prev = 0;
double gun_set = 90;   //for gun pitch set condition
double gun_prev = 90;

void setup() {
  // Sets pins Mode
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  servo_cam.attach(servoCAM);
  servo_tilt.attach(servoTILT);
  servo_gun.attach(servoGUN);
  
  Serial.begin(9600); // serial comm to raspi
  //Serial.print("Setup success");
}
void loop() {
  if (Serial.available())
  {
    get_setpoint();
    move_all();   // move all actuator
  }
}


void get_setpoint()   //get setpoint from Server, through serial comms
{
  String serialResponse;
  serialResponse = Serial.readStringUntil('\0');
  
  char buf[sizeof(serialResponse)];
  serialResponse.toCharArray(buf, sizeof(buf));
  char *p = buf;
  char *camc, *gunc;  //string variable for cam and gun
  
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
      //cam_set = double(camc);
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
      Serial.println(yaw_set);
      str = strtok_r(p, ",", &p);
    }else if(str[0]=='g')         // get gun setpoint
    {
      gunc = &str[1];
      String s=gunc;
      gun_set = (double) s.toFloat();
      Serial.print("gun_set = ");
      Serial.println(gun_set);
    }
}

void move_cam(double degree)
{
  double cam_setd += 90;  //map to servo degree between 0-180
  servo_cam.write(cam_setd);
  cam_prev = cam_setd;
}

void move_tilt(double degree)
{
  double tilt_setd += 90;  //map to servo degree between 0-180
  servo_tilt.write(tilt_setd);
  tilt_prev = tilt_setd;
}

double move_stepper(double degree)
{
  
  double deg = degree;
  
  //clockwise is positive
  if (degree > 0)
  {
    digitalWrite(dirPin,HIGH);
  }else
  {
    digitalWrite(dirPin,LOW);
    deg = -deg;
  }
  
  if (degree!=0)
  {
    int step_req = (int) (deg/step_RES);
    for(int x = 0; x <= step_req; x++) {  //give pulse until degree achieved
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(500); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(500); 
    } 
  }
  
  yaw_set +=degree;
  yaw_prev = yaw_set;
  delay(1);
}

void move_gun(double degree)
{
  double gun_setd += 90;  //map to servo degree between 0-180
  servo_gun.write(gun_setd);
  gun_prev = gun_set;
}

void move_all()
{
  if(cam_set != cam_prev) //if the value change
  {
    move_cam(cam_set);
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

