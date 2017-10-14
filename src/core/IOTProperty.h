/*
** EasyIOT - Device Property Class
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
#ifndef _IOT_PROPERTY_H
#define _IOT_PROPERTY_H

#include <Arduino.h>
#include <time.h>
#include <sys/time.h>
#include <esp_log.h>
#include "core/IOTStrings.h"
#include "core/IOTTimer.h"

#undef min
#define min(a, b) ((a) < (b) ? (a) : (b))

/*
** Forward References
*/
class IOTFunction;
class IOTMaster;

/*
** Useful Constants
*/
#define CONSTANT_GRAVITY_EARTH (9.80665F) /**< Earth's gravity in m/s^2 */
#define CONSTANT_GRAVITY_MOON (1.6F)      /**< The moon's gravity in m/s^2 */
#define CONSTANT_GRAVITY_SUN (275.0F)     /**< The sun's gravity in m/s^2 */
#define CONSTANT_GRAVITY_STANDARD (CONSTANT_GRAVITY_EARTH)
#define CONSTANT_MAGFIELD_EARTH_MAX (60.0F)      /**< Maximum magnetic field on Earth's surface */
#define CONSTANT_MAGFIELD_EARTH_MIN (30.0F)      /**< Minimum magnetic field on Earth's surface */
#define CONSTANT_PRESSURE_SEALEVELHPA (1013.25F) /**< Average sea level pressure is 1013.25 hPa */
#define CONSTANT_DPS_TO_RADS (0.017453293F)      /**< Degrees/s to rad/s multiplier */
#define CONSTANT_GAUSS_TO_MICROTESLA (100)       /**< Gauss to micro-Tesla multiplier */

/*
** Equates and Defintions
*/
#define IOT_MAX_SSID 31
#define IOT_MAX_PASS 63
#define IOT_MAX_HOST 62

#define IOTPROPERTY_MAX_LABEL 48
#define IOTPROPERTY_MAX_PREFIX 32
#define IOTPROPERTY_MAX_SUFFIX 32

/*
** Flags (check iotPIN class for special usage)
*/
#define IOT_FLAG_UKNOWN 0x0000
#define IOT_FLAG_SENSOR 0x0001
#define IOT_FLAG_CONTROL 0x0002
#define IOT_FLAG_READONLY 0x0008
#define IOT_FLAG_VOLATILE 0x0010
#define IOT_FLAG_INVERT 0x0100      // Used by IOTPIN
#define IOT_FLAG_SYSTEM 0x0200
#define IOT_FLAG_CONFIG 0x0400
#define IOT_FLAG_LOCK_LABEL 0x0800
#define IOT_FLAG_DISABLED 0x1000
#define IOT_FLAG_RESTART 0x8000

/*
** Property (Data) Type - This Matches the UPnP 1.1 Device Architecture Data Types
*/
enum class PROPERTY_TYPE
{
  // Align with SmartThings Capabilities Data Types
  //
  STRING,
  NUMBER,
  VECTOR, // ST = VECTOR3 (x,y,z)
  ENUM,
  DNUM,  // ST = DYNAMIC_ENUM
  COLOR, // ST = COLOR_MAP [hue: 50, saturation: 75]
  JSON,  // ST = JSON_OBJECT
  DATE,

  // Additional Data Types
  //
  BOOL,  // Boolean (map to ST number)
  STREAM // Stream of data bytes
};

/*
** Property Class
*/
enum class PROPERTY_CLASS
{
  GENERIC = (0),
  MODE,
  TIME,
  TIMEZONE,
  MSTIMER,
  IPHOST,
  IP4ADDRESS,

  ACCELEROMETER,
  AMBIENT_TEMPERATURE,
  BAROMETER,
  BRIGHTNESS,
  COLOR,
  CURRENT,
  EFFECT,
  FREQUENCY,
  HUE,
  ILLUMINANCE,
  MAGNETIC_FIELD,
  PRESSURE,
  PROXIMITY,
  RELATIVE_HUMIDITY,
  VOLTAGE,

  // Special: Boolean - also see IOTPIN.h
  BOOL = (0x1000),
  BOOLEAN,
  MOTION,
  LOGIC,  
  SWITCH,
  OUTLET,
  RELAY,
  VALVE,

  // Special: Lights
  LIGHT = (0x2000 | SWITCH),
  LIGHT_DIMMER,
  LIGHT_HSB  
};

/*
** Base Property Class
*/
class IOTProperty
{
public:
  friend class IOTFunction;
  friend class IOTMaster;
  
  inline bool isReadOnly() { return _dataFlags & IOT_FLAG_READONLY; }
  inline time_t timeStamp() { return _dataTime; }

  inline size_t dataLen(void) { return _dataLen; }
  inline PROPERTY_TYPE dataType(void) { return _dataType; }
  inline PROPERTY_CLASS dataClass(void) { return _dataClass; }

  String dataPrefix(void);
  String dataSuffix(void);
  String dataLabel(void);

  void getDataLabel(char *s);
  void setDataLabel(const char *s, bool lock = false);

  virtual String getData(void) = 0;
  char *getData(char *, size_t max);
  bool setData(const char *newVal, bool urgent = false); 
  bool setData(String &newVal, bool urgent = false);

protected:
  IOTProperty(IOTFunction *IOTFunction, uint16_t flags, size_t dataLen, PROPERTY_TYPE pType,
              PROPERTY_CLASS pClass, const char *prefix = NULL, const char *suffix = NULL, const char *label = NULL);

  virtual void _setClass(PROPERTY_CLASS pClass);
  virtual void *_dataPtr(void) = 0;
  virtual bool _dataSet(String &newVal) = 0;

  IOTFunction *_IOTFunction;
  PROPERTY_TYPE _dataType;
  PROPERTY_CLASS _dataClass;
  char *_dataLabel;
  const char *_dataPrefix;
  const char *_dataSuffix;
  size_t _dataLen;
  uint16_t _dataFlags;
  time_t _dataTime;
};

/*
** Timer Property
*/
class IOTPropertyTimer : public IOTProperty, public IOTTimer
{
public:
  friend class IOT;
  friend class IOTFunction;

  IOTPropertyTimer(IOTFunction *IOTFunction, uint16_t flags, uint32_t timer)
      : IOTTimer(timer),
        IOTProperty(IOTFunction, flags, sizeof(uint32_t), PROPERTY_TYPE::NUMBER, PROPERTY_CLASS::MSTIMER)
  {
  }

protected:
  void *_dataPtr(void) { return (void *)&timerDuration; }
};

/*
** String Property
*/
class IOTPropertyString : public IOTProperty
{
public:
  friend class IOT;
  friend class IOTFunction;

  IOTPropertyString(IOTFunction *IOTFunction, uint16_t flags, const char *defVal,
                    size_t maxLen, PROPERTY_CLASS pClass = PROPERTY_CLASS::GENERIC,
                    const char *prefix = NULL, const char *suffix = NULL, const char *label = NULL)
      : IOTProperty(IOTFunction, flags, maxLen, PROPERTY_TYPE::STRING, pClass, prefix, suffix, label)
  {    
    _dataLen = min(maxLen, 256L);
    _dataVal = (char *)malloc(maxLen + 1);   
    (void)_dataSet((char *)defVal);   
  }

  String getData(void) { return String(_dataVal); }
  
protected:
  void *_dataPtr(void) { return (void *)_dataVal; }
  bool _dataSet(String &newVal) { return _dataSet((char *)newVal.c_str()); }

  bool _dataSet(char *newVal)
  {
    bool changed = false;

    if (_dataVal != NULL)
    {
      _dataVal[_dataLen] = '\0';

      if (newVal == NULL)
      {
        if ((changed = strncmp(_dataVal, strNull, _dataLen)))
          memset(_dataVal, 0, _dataLen);
      }
      else if ((changed = strncmp(_dataVal, (const char *)newVal, _dataLen)))
        strncpy(_dataVal, (const char *)newVal, _dataLen);
    }

    return changed;
  }

  char *_dataVal;
};

/*
** Number Property
*/
template <class _T>
class IOTPropertyNumber : public IOTProperty
{
public:
  friend class IOT;
  friend class IOTFunction;

  IOTPropertyNumber(IOTFunction *IOTFunction, uint16_t flags, _T &dataRef, _T dataMin, _T dataMax,
                    PROPERTY_CLASS pClass, const char *prefix = NULL, const char *suffix = NULL, const char *label = NULL)
      : _dataRef(dataRef), _dataMin(dataMin), _dataMax(dataMax),
        IOTProperty(IOTFunction, flags, sizeof(_T), PROPERTY_TYPE::NUMBER, pClass, prefix, suffix, label)
  {
  }

  String getData(void) { return _stringify(_dataRef); }
  
protected:
  void *_dataPtr(void) { return (void *)&_dataRef; }

  virtual bool _dataSet(String &newVal)
  {
    _T nv = _T(0);

    _destring(newVal, &nv);
    return _dataSet(nv);
  }

  bool _dataSet(_T &newVal)
  {
    bool changed = false;

    if (newVal > _dataMax)
      newVal = _dataMax;
    if (newVal < _dataMin)
      newVal = _dataMin;

    if (newVal != _dataRef)
    {
      _dataRef = newVal;
      return true;
    }
    return false;
  }

  _T &_dataRef;
  _T _dataMin;
  _T _dataMax;

private:
  void _destring(String &val, bool *nv) { *nv = (bool)(val.toInt() & 0x01L); }
  void _destring(String &val, int8_t *nv) { *nv = (int8_t)val.toInt(); }
  void _destring(String &val, uint8_t *nv) { *nv = (uint8_t)val.toInt(); }
  void _destring(String &val, int16_t *nv) { *nv = (int16_t)val.toInt(); }
  void _destring(String &val, uint16_t *nv) { *nv = (uint16_t)val.toInt(); }
  void _destring(String &val, int32_t *nv) { *nv = (int32_t)val.toInt(); }
  void _destring(String &val, uint32_t *nv) { *nv = (uint32_t)val.toInt(); }
  void _destring(String &val, float *nv) { *nv = val.toFloat(); }
  void _destring(String &val, double *nv) { *nv = (double)val.toInt(); }

  String _stringify(bool t) { return String((t) ? "1" : "0"); }
  String _stringify(int8_t t) { return String(t); }
  String _stringify(int16_t t) { return String(t); }
  String _stringify(int32_t t) { return String(t); }
  String _stringify(uint8_t t) { return String(t); }
  String _stringify(uint16_t t) { return String(t); }
  String _stringify(uint32_t t) { return String(t); }
  String _stringify(float t) { return String(t, 2); }
  String _stringify(double t) { return String(t, 4); }
};

// TODO: Add logic inversion flag
//
class IOTPropertyBool : public IOTPropertyNumber<bool>
{
public:
  IOTPropertyBool(IOTFunction *IOTFunction, uint16_t flags, bool &dataRef,
                  PROPERTY_CLASS pClass, const char *prefix = NULL, const char *suffix = NULL, const char *label = NULL)
      : IOTPropertyNumber(IOTFunction, flags, dataRef, 0, 1, pClass, prefix, suffix, label)
  {
    _dataType = PROPERTY_TYPE::BOOL;
  }

  String getData(void)
  {
    switch (_dataClass) {
      case PROPERTY_CLASS::LOGIC: return String((_dataRef) ? strHigh : strLow);       
      case PROPERTY_CLASS::LIGHT:
      case PROPERTY_CLASS::SWITCH: return String((_dataRef) ? strOn : strOff); 
      case PROPERTY_CLASS::MOTION:
      case PROPERTY_CLASS::BOOLEAN: String((_dataRef) ? strTrue : strFalse);
      case PROPERTY_CLASS::RELAY: String((_dataRef) ? strClosed : strOpen);
      case PROPERTY_CLASS::VALVE: String((_dataRef) ? strOpen : strClosed);
      default:
        break;
    }

    return String((_dataRef) ? "1" : "0");
  }
};

template <>
class IOTPropertyNumber<char *> : public IOTPropertyString
{
};

#endif // _IOT_PROPERTY_H

/*

class IOTPropertySwitch : public IOTPropertyNumber<bool>
{
  String _dataStr(void) { return String((_dataRef) ? strOn : strOff); }  
};

class IOTPropertyPercent : public IOTPropertyNumber<float>
{
};



class IP4AddressProperty : public IOTPropertyBase
    {
      public:
        friend class IOT;
        friend class IOTFunction;

        IP4AddressProperty(IOTFunction *func, IPAddress addr, bool readOnly = true, 
          const char *prefix = NULL, const char *suffix = NULL)
        {
          IP4AddressProperty(func, (uint32_t)0, readOnly, prefix, suffix);
          _dataVal = addr;
        }

        IP4AddressProperty(IOTFunction *func, const char *addr, bool readOnly = true, 
          const char *prefix = NULL, const char *suffix = NULL)
        {
          IP4AddressProperty(func, (uint32_t)0, readOnly, prefix, suffix);
          _dataVal.fromString(addr);
        }

        IP4AddressProperty(IOTFunction *func, uint32_t addr, bool readOnly = true, 
          const char *prefix = NULL, const char *suffix = NULL)
        {          
          _flags = readOnly ? IOT_FLAG_READONLY : 0;          
          _IOTFunction = func;
          _dataClass = PROPERTY_CLASS::CLASS_IP4ADDRESS;
          _dataLabel = NULL;
          _dataPrefix = prefix;
          _dataSuffix = suffix;
          _dataLen = sizeof(addr);
          _dataVal = addr;
        }

      protected:
        void *_getDataVal()
        {
          return (void *)&_dataVal;
        }
        
        bool _setDataVal(void *vp)
        {
          bool changed = false;

          if (vp != NULL) {
              changed = _dataVal != *(IPAddress *)vp; 
              _dataVal = *(IPAddress *)vp;
          }

          return changed;
        }

        IPAddress _dataVal;
    };

    template <class _T>
    class IOTProperty : public IOTPropertyBase
    {
      public:
        friend class IOT;
        friend class IOTFunction;

        IOTProperty(IOTFunction *func, _T data, _T min, _T max, _T res, 
            PROPERTY_CLASS pClass = PROPERTY_CLASS::CLASS_GENERIC, bool readOnly = true)
        {
            _setClass(pClass);
            _flags = readOnly ? IOT_FLAG_READONLY : 0;
            _IOTFunction = func;
            _dataLabel = NULL;
            _dataPrefix = NULL;
            _dataSuffix = NULL;
            _dataVal = data;
            _dataLen = sizeof(_T);
        }
      
      protected:
        void *_getDataVal(void) 
        {
            return (void *)&_dataVal;
        }
        
        bool _setDataVal(void *vp) 
        {
            bool changed = false;
           _dataVal = *(_T *)vp;
           return changed; 
        }

        const char *_stringify(bool t) { return (t) ? strTrue : strFalse; }
        const char *_stringify(int8_t t) { return PRIi8; }
        const char *_stringify(int16_t t) { return PRIi16; }
        const char *_stringify(int32_t t) { return PRIi32; }
        const char *_stringify(uint8_t t) { return PRIu8; }
        const char *_stringify(uint16_t t) { return PRIu16; }
        const char *_stringify(uint32_t t) { return PRIu32; }
        const char *_stringify(float t) { return "%.2f"; }
        const char *_stringify(double t) { return "%.4f"; }
        const char *_stringify(char *) { return "%s"; }
      
        _T _dataVal;
        _T _dataMin;
        _T _dataMax;
        _T _dataRes;
    };

*/
/******************************************************************************/