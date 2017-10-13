/*
** EasyIOT - Wire (I2C) Function (Abstract) Class
**
** This program is provided free for you to use in any way that you wish,
** subject to the laws and regulations where you are using it.  Due diligence
** is strongly suggested before using this code.  Please give credit where due.
**
** The Author makes no warranty of any kind, express or implied, with regard
** to this program or the documentation contained in this document.  The
** Author shall not be liable in any event for incidental or consequential
** damages in connection with, or arising out of, the furnishing, performance
** or use of these programs.
*/
#ifndef _IOT_WIRE_H
#define _IOT_WIRE_H

#include "IOTFunction.h"
#include "iotTimer.h"
#include <Arduino.h>
#include <Wire.h>

class IOTWire : public IOTFunction
{
public:
  inline void i2cWire(TwoWire *wireLib) { pWire = wireLib; }

protected:
  IOTWire(const char *tag, uint8_t numProperties, uint8_t addr, TwoWire *wireLib = &Wire)
      : IOTFunction(_i2cTag, numProperties), _i2c_address(addr), pWire(wireLib)
  {
    memset(_i2cTag, 0, sizeof(_i2cTag));
    sprintf(_i2cTag, "%.*s/%2.2X", IOTFunction_MAX_TAG, tag, _i2c_address);
  }

  uint8_t _i2c_address;
  char _i2cTag[16];

private:
  TwoWire *pWire;
};

#endif // _IOTI2C_H
/******************************************************************************/
