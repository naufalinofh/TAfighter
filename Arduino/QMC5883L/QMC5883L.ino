#include <Wire.h> //I2C Arduino Library

#define addr 0x0D //I2C Address for The HMC5883
const float declinationAngle = 0.012; // Find yours here: http://www.magnetic-declination.com. Bandung is: 0*42' E --> (0*deg + 42/60)*PI/180 = 0.012 radians


void setup() {

  Serial.begin(9600);
  Wire.begin();


  Wire.beginTransmission(addr); //start talking
  Wire.write(0x0B); // Tell the HMC5883 to Continuously Measure
  Wire.write(0x01); // Set the Register
  Wire.endTransmission();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x09); // Tell the HMC5883 to Continuously Measure
  Wire.write(0x1D); // Set the Register
  Wire.endTransmission();
}

void loop() {

  int x, y, z; //triple axis data

  //Tell the HMC what regist to begin writing data into


  Wire.beginTransmission(addr);
  Wire.write(0x00); //start with register 3.
  Wire.endTransmission();

  //Read the data.. 2 bytes for each axis.. 6 total bytes
  Wire.requestFrom(addr, 6);
  if (6 <= Wire.available()) {
    x = Wire.read(); //MSB  x
    x |= Wire.read() << 8; //LSB  x
    z = Wire.read(); //MSB  z
    z |= Wire.read() << 8; //LSB z
    y = Wire.read(); //MSB y
    y |= Wire.read() << 8; //LSB y
  }
  Serial.print("mx: ");
  Serial.print(x);
   Serial.print("\t my: ");
  Serial.print(y);
   Serial.print("\t mz: ");
  Serial.print(z);
  float heading = atan2(y, x);
  
    heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI;
  
  
  // Show Values
  Serial.print("heading: ");
  Serial.println(headingDegrees);

  delay(1000);
}
