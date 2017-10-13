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
#include "IOTProperty.h"
#include "IOTFunction.h"
#include <esp_log.h>

IOTProperty::IOTProperty(IOTFunction *IOTFunction, uint16_t flags, size_t dataLen,
                         PROPERTY_TYPE pType, PROPERTY_CLASS pClass, const char *prefix, 
                         const char *suffix, const char *label)
    : _IOTFunction(IOTFunction), 
    _dataLen(dataLen), 
    _dataType(pType), 
    _dataClass(pClass),
    _dataPrefix(prefix), 
    _dataSuffix(suffix), 
    _dataLabel(NULL),
    _dataTime(0)
{
    if (prefix == NULL && suffix == NULL)
        _setClass(pClass);
    setDataLabel(label);
    _dataFlags = flags;
}

/*
** Value Getter
*/
char *IOTProperty::getData(char *buf, size_t len)
{
    String value = getData();
    snprintf(buf, len, value.c_str());
    return buf;
}

/*
** Value Setter
*/
bool IOTProperty::setData(const char *newVal, bool urgent)
{
    String nv(newVal);

    return setData(nv, urgent); 
}

bool IOTProperty::setData(String &newVal, bool urgent)
{
    bool changed = false;

    if (!(_dataFlags & IOT_FLAG_READONLY))
    {
        if ((changed = _dataSet(newVal)))      
            time(&_dataTime);
    
        if (changed && _IOTFunction != NULL)
        {
            if (_IOTFunction->_propUpdate(this))
                _IOTFunction->_postUpdate(this, urgent);
        }
    }

    return changed;
}

/*
** Property Class
*/
void IOTProperty::_setClass(PROPERTY_CLASS pClass)
{
    /*
    _dataPrefix = _strPrefix[0]; // Generic Value
    _dataSuffix = _strSuffix[0]; // Empty

    switch (pClass)
    {
    case PROPERTY_CLASS::CLASS_SWITCH:
        _dataPrefix = _strPrefix[1];
        _dataSuffix = _strSuffix[0];
        return;
    case PROPERTY_CLASS::CLASS_AMBIENT_TEMPERATURE:
        _dataPrefix = _strPrefix[2];
        _dataSuffix = _strSuffix[1];
        return;
    case PROPERTY_CLASS::CLASS_RELATIVE_HUMIDITY:
        _dataPrefix = _strPrefix[3];
        _dataSuffix = _strSuffix[2];
        return;
    case PROPERTY_CLASS::CLASS_ILLUMINANCE:
        _dataPrefix = _strPrefix[4];
        _dataSuffix = _strSuffix[3];
        return;
    case PROPERTY_CLASS::CLASS_BAROMETER:
        _dataPrefix = _strPrefix[5];
        _dataSuffix = _strSuffix[4];
        return;
    case PROPERTY_CLASS::CLASS_PRESSURE:
        _dataPrefix = _strPrefix[6];
        _dataSuffix = _strSuffix[4];
        return;
    }
    */
}

/*
** String Getters and Setters
*/
String IOTProperty::dataPrefix(void)
{
    return String(_dataPrefix);
}

String IOTProperty::dataSuffix(void)
{
    return String(_dataSuffix);
}

String IOTProperty::dataLabel(void)
{
    return String(_dataLabel);
}

void IOTProperty::getDataLabel(char *s)
{
    if (s != NULL)
    {
        int n = 0;

        if (_dataLabel != NULL)
        {
            n = IOTPROPERTY_MAX_LABEL;
            strncpy(s, _dataLabel, n);
        }
        s[n] = '\0';
    }
}

void IOTProperty::setDataLabel(const char *s, bool lock)
{
    if (_dataFlags & IOT_FLAG_LOCK_LABEL)
        return;

    if (_dataLabel != NULL)
    {
        free(_dataLabel);
        _dataLabel = NULL;
    }

    if (s != NULL)
    {
        int n = strlen(s);
        if (n > 0)
        {
            n = std::min(n, IOTPROPERTY_MAX_LABEL);
            if ((_dataLabel = (char *)malloc(n + 1)) != NULL)
            {
                memcpy(_dataLabel, s, n);
                _dataLabel[n] = '\0';

                if (!lock)
                {
                    if (_IOTFunction != NULL)
                    {
                        for (uint8_t p = 0; _IOTFunction->_Properties != NULL && p < _IOTFunction->_propCount; p++)
                        {
                            if (_IOTFunction->_Properties[p] == this)
                            {
                                char key[16];

                                sprintf(key, "%s@P%3.3d", strLabel, p);
                                (void)_IOTFunction->_saveChars(key, _dataLabel);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (lock)
        _dataFlags |= IOT_FLAG_LOCK_LABEL;
}
/******************************************************************************/