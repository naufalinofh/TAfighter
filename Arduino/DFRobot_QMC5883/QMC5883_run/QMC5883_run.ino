/*!
 * @file QMC5883_compass.cpp
 * @brief The program shows how to realize the function compass.When the program runs, please spin QMC5883 freely to accomplish calibration.
 * @n 3-Axis Digital Compass IC
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2017
 * @copyright	GNU Lesser General Public License
 *
 * @author [dexian.huang](952838602@qq.com)
 * @version  V1.0
 * @date  2017-7-3
 */

#include <Wire.h>
#include "QMC5883_run.h"

QMC5883 compass;
//CONSTANT
const float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);   // You can find your declination on: http://magnetic-declination.com/... For Bandungdeclination angle is 4'26E (positive)
  

//VARIABLE
float head;

void setup()
{
  Serial.begin(9600);
  while (!compass.begin())
  {
    Serial.println("Could not find a valid QMC5883 sensor, check wiring!");
    delay(500);
  }
}

void loop()
{
  head = getHeading();
  delay(1000);
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

  // Correct for heading < 0deg and heading > 360deg
  if (heading < -PI){
    heading += 2 * PI;
  }

  if (heading > PI){
    heading -= 2 * PI;
  }

  // Convert to degrees
  heading = heading * 180/PI; 

  // Output
  Serial.print(" Heading = ");
  Serial.println(heading);

  
  delay(10);
  return heading;
}

