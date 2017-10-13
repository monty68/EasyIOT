/*
** EasyIOT - BH1750(FVI) Digital Light Sensor I2C Device
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
#ifndef _BH1750_H
#define _BH1750_H

#include "core/IOTWire.h"
#include "core/IOTSmooth.h"

#define I2C_ADDRESS_BH1750 0x23

class BH1750 : public IOTWire
{
  public:
    float avgReading(void) { return _avgReading; }
    uint16_t rawReading(void) { return _rawReading; }

    BH1750(uint8_t addr = I2C_ADDRESS_BH1750) : IOTWire("BH1750", 3, addr), _mode(3)
    {
        /*
        IOTPropertyNumber(IOTFunction *IOTFunction, uint16_t flags, _T &dataRef, _T dataMin, _T dataMax,
            PROPERTY_CLASS pClass, const char *prefix = NULL, const char *suffix = NULL, const char *label = NULL)
        */

        _Properties[0] = new IOTPropertyNumber<uint8_t>(this, 0, _mode, 0, 6, PROPERTY_CLASS::MODE);        
        //_Properties[1] = new IOTPropertyTimer(this, 0, 30 * 1000);
        //_Properties[2] = new IOTPropertyNumber<float>(this, IOT_FLAG_READONLY, PROPERTY_CLASS::CLASS_ILLUMINANCE, _avgReading);
    }

  protected:
    void iotStartup(void)
    {
        if (_state == IOT_RUNNING)
            return;
        Serial.printf("BH1750 Mode: %d (%s)", _mode, _Properties[0]->getData().c_str());
        //_setMode(mode);
        _state = IOT_RUNNING;
    }

    void iotShutdown(void)
    {
        if (_state != IOT_RUNNING)
            return;
        //_setMode(0);
        _state = IOT_STOPPED;
    }

    void iotService(void)
    {
        if (_state != IOT_RUNNING)
            return;

        /*
        IOTPropertyTimer *pollTime = (IOTPropertyTimer *)_Properties[1];
    
        if (pollTime->expired()) {
    
            _rawReading = 23;
    
            float newReading = luxSmooth.smooth((float)(_rawReading / 1.2));
            if (newReading != _avgReading)
                _postUpdate(_Properties[2]);
        }
        */
    }

    bool _propUpdate(IOTProperty *prop)
    {
    }

  private:
    uint8_t _mode;
    float _avgReading;
    uint16_t _rawReading;
    IOTSmooth<float> luxSmooth;
};

#endif // _BH1750_H
/******************************************************************************/
