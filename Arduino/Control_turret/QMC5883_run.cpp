/*!
 * @file DFRobot_QMC5883.cpp
 * @brief Compatible with QMC5883 and QMC5883
 * @n 3-Axis Digital Compass IC
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2017
 * @copyright	GNU Lesser General Public License
 *
 * @author [dexian.huang](952838602@qq.com)
 * @version  V1.0
 * @date  2017-7-3
 */

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include "QMC5883_run.h"



bool QMC5883::begin()
{
  int retry;
  retry = 2;
  
    while(retry--){
      Wire.begin();
      Wire.beginTransmission(QMC5883_ADDRESS);
      isQMC_ = (0 == Wire.endTransmission());
      delay(20);
    }
    
    if ((fastRegister8(QMC5883_REG_IDENT_B) != 0x01)
      || (fastRegister8(QMC5883_REG_IDENT_C) != 0x40)
      || (fastRegister8(QMC5883_REG_IDENT_D) != 0x01)){
        return false;
    }
    mgPerDigit = 4.35f;
    return true;
  if(! isQMC_){
    return false;
  }
}

void QMC5883::calibrate()
{
  if(v.XAxis < minX ) minX = v.XAxis;
  if(v.XAxis > maxX ) maxX = v.XAxis;
  if(v.YAxis < minY ) minY = v.YAxis;
  if(v.YAxis > maxY ) maxY = v.YAxis;
  if(v.ZAxis < minZ ) minZ = v.ZAxis;
  if(v.ZAxis > maxZ ) maxZ = v.ZAxis;
}

void QMC5883::initMinMax()
{
  minX = v.XAxis;
  maxX = v.XAxis;
  minY = v.YAxis;
  maxY = v.YAxis;
  minZ = v.ZAxis;
  maxZ = v.ZAxis;
}

Vector QMC5883::readNormalize(void)
{
  int range = 10;
  float Xsum = 0.0;
  float Ysum = 0.0;
  float Zsum = 0.0;
    while (range--){
      v.XAxis = ((float)readRegister16(QMC5883_REG_OUT_X_M)) * mgPerDigit;
      v.YAxis = ((float)readRegister16(QMC5883_REG_OUT_Y_M)) * mgPerDigit;
      v.ZAxis = (float)readRegister16(QMC5883_REG_OUT_Z_M) * mgPerDigit;
      Xsum += v.XAxis;
      Ysum += v.YAxis;
      Zsum += v.ZAxis;
    }
    v.XAxis = Xsum/range;
    v.YAxis = Ysum/range;
    v.ZAxis = Zsum/range;
        
    calibrate();
    v.XAxis= map(v.XAxis,minX,maxX,-360,360);
    v.YAxis= map(v.YAxis,minY,maxY,-360,360);
    v.ZAxis= map(v.ZAxis,minZ,maxZ,-360,360);
  
  return v;
}

// Read byte to register
uint8_t QMC5883::fastRegister8(uint8_t reg)
{
  uint8_t value=0;
  Wire.beginTransmission(QMC5883_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();
    Wire.requestFrom(QMC5883_ADDRESS, 1);
    #if ARDUINO >= 100
        value = Wire.read();
    #else
        value = Wire.receive();
    #endif
    Wire.endTransmission();
  return value;
}

// Read word from register
int16_t QMC5883::readRegister16(uint8_t reg)
{
  int16_t value=0;
    Wire.beginTransmission(QMC5883_ADDRESS);
    #if ARDUINO >= 100
        Wire.write(reg);
    #else
        Wire.send(reg);
    #endif
    Wire.endTransmission();
    Wire.beginTransmission(QMC5883_ADDRESS);
    Wire.requestFrom(QMC5883_ADDRESS, 2);
    while(!Wire.available()) {};
    #if ARDUINO >= 100
        uint8_t vha = Wire.read();
        uint8_t vla = Wire.read();
    #else
        uint8_t vha = Wire.receive();
        uint8_t vla = Wire.receive();
    #endif
    Wire.endTransmission();
    value = vha << 8 | vla;
  
  return value;
}
