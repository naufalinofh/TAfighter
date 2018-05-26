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
#define QMC5883_REG_OUT_X_L          (0x00)
#define QMC5883_REG_OUT_Z_M          (0x05)
#define QMC5883_REG_OUT_Z_L          (0x04)
#define QMC5883_REG_OUT_Y_M          (0x03)
#define QMC5883_REG_OUT_Y_L          (0x02)
#define QMC5883_REG_STATUS           (0x06)
#define QMC5883_REG_CONFIG_1         (0x09)
#define QMC5883_REG_CONFIG_2         (0x0A)
#define QMC5883_REG_IDENT_B          (0x0B)
#define QMC5883_REG_IDENT_C          (0x20)
#define QMC5883_REG_IDENT_D          (0x21)


typedef enum
{
  
  QMC5883_SAMPLES_8     = 0b11,
  QMC5883_SAMPLES_4     = 0b10,
  QMC5883_SAMPLES_2     = 0b01,
  QMC5883_SAMPLES_1     = 0b00
} QMC5883_samples_t;

typedef enum
{
  QMC5883_DATARATE_10HZ        = 0b00,
  QMC5883_DATARATE_50HZ        = 0b01,
  QMC5883_DATARATE_100HZ       = 0b10,
  QMC5883_DATARATE_200HZ       = 0b11
} QMC5883_dataRate_t;

typedef enum
{
  QMC5883_RANGE_2GA     = 0b00,
  QMC5883_RANGE_8GA     = 0b01
} QMC5883_range_t;

typedef enum
{
  QMC5883_SINGLE        = 0b00,
  QMC5883_CONTINOUS     = 0b01
} QMC5883_mode_t;


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

  Vector readRaw(void);
  Vector readNormalize(void);
  
  void initMinMax();
  void calibrate(void);
  
  void  setRange(QMC5883_range_t range);
  QMC5883_range_t getRange(void);

  void  setMeasurementMode(QMC5883_mode_t mode);
  QMC5883_mode_t getMeasurementMode(void);

  void  setDataRate(QMC5883_dataRate_t dataRate);
  QMC5883_dataRate_t getDataRate(void);

  void  setSamples(QMC5883_samples_t samples);
  QMC5883_samples_t getSamples(void);
  
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
