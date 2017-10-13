/*
** EasyIOT - INA3221 Triple Channel Current/Voltage Monitor I2C Device
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
#ifndef _INA3221_H
#define _INA3221_H

#include "core/IOTWire.h"
#define I2C_ADDRESS_INA3221_0 (0x40) // A0 to GND
#define I2C_ADDRESS_INA3221_1 (0x41) // A0 to Vs+
#define I2C_ADDRESS_INA3221_2 (0x42) // A0 to SDA
#define I2C_ADDRESS_INA3221_3 (0x43) // A0 to SCL

class INA3221 : public IOTWire
{
  public:
    INA3221(uint8_t addr = I2C_ADDRESS_INA3221_0) : IOTWire("INA3221", 3, addr)
    {
        //_Properties[0] = new IOTProperty<float>(this, 0);
        //_Properties[1] = new IOTProperty<uint8_t>(this, 1);
        //_Properties[2] = new IOTProperty<uint32_t>(this, 30 * 1000);
    }

  protected:
    void iotStartup(void)
    {
    }

    void iotShutdown(void)
    {
    }

    void iotService(void)
    {
    }

    bool _propUpdate(IOTProperty *prop)
    {
        return true;
    }
};

#endif // _INA3221_H
/******************************************************************************/
