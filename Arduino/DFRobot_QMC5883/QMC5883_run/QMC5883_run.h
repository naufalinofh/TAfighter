/*!
 * @file DFRobot_QMC5883.h
 * @brief Compatible with QMC5883 and QMC5883
 * @n 3-Axis Digital Compass IC
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2017
 * @copyright	GNU Lesser General Public License
 *
 * @author [dexian.huang](952838602@qq.com)
 * @version  V1.0
 * @date  2017-7-3
 * edited by Naufalino Fadel Hutomo, vMarch2018
 */

#ifndef QMC5883_H
#define QMC5883_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define QMC5883_ADDRESS              (0x0D)
#define QMC5883_REG_OUT_X_M          (0x01)
#define QMC5883_REG_OUT_Z_M          (0x05)
#define QMC5883_REG_OUT_Y_M          (0x03)
#define QMC5883_REG_IDENT_B          (0x0B)
#define QMC5883_REG_IDENT_C          (0x20)
#define QMC5883_REG_IDENT_D          (0x21)



#ifndef VECTOR_STRUCT_H
#define VECTOR_STRUCT_H
struct Vector
{
  float XAxis;
  float YAxis;
  float ZAxis;
};
#endif

class QMC5883
{
public:
  QMC5883():isQMC_(false),minX(0),maxX(0),minY(0), maxY(0), minZ(0), maxZ(0),firstRun(true)
    {}
  bool begin(void);

  Vector readNormalize(void);
  
  void initMinMax();
  void calibrate(void);
  
private:
  float mgPerDigit;
  Vector v;
  bool isQMC_;
    
  float minX, maxX;
  float minY, maxY;
  float minZ, maxZ;
  bool firstRun;
  uint8_t fastRegister8(uint8_t reg);
  int16_t readRegister16(uint8_t reg);
};

#endif
