/*
** EasyIOT - Pin Function Class
**
** Debounce code based on work by Thomas Fredericks, Copyright (c) 2013 
** https://github.com/thomasfredericks/Bounce2
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
#ifndef _IOT_PIN_H
#define _IOT_PIN_H

#include "core/IOTFunction.h"

/*
** General Defintions and Equates
*/
#define PIN_STATE_DEBOUNCED 0
#define PIN_STATE_UNSTABLE  1
#define PIN_STATE_CHANGED   3
#define IOT_PIN_VIRTUAL    0x80

#ifndef _BV
#define _BV(n) (1<<(n))
#endif

enum class PIN_CLASS {
    BOOL = PROPERTY_CLASS::BOOL,
    BOOLEAN,
    MOTION,
    LOGIC,  
    SWITCH,
    OUTLET,
    RELAY,
    VALVE  
};

enum class PIN_DEBOUNCE {
    OFF,
    STABLE,
    PROMPT,
    LOCKOUT,
};

/*
** PIN Function Class
*/
class IOTPIN : private IOTTimer, public IOTFunction, public IOTPropertyBool
{
  public:
    IOTPIN(uint8_t pin, uint8_t mode = INPUT, PIN_CLASS pClass = PIN_CLASS::LOGIC, 
        const char *prefix = NULL, const char *suffix = NULL, const char *label = NULL)
        : IOTTimer(0), 
        IOTFunction(_pinTag, 1), 
        IOTPropertyBool(this, 0, _pinState, (PROPERTY_CLASS)pClass, prefix, suffix, label),
        _pin(pin),
        _pinMode(mode),
        _pinState(0),
        _safeMode(OPEN_DRAIN),
        _dbState(0),
        _dbMode(PIN_DEBOUNCE::OFF)
    {
        memset(_pinTag, 0, sizeof(_pinTag));
        snprintf(_pinTag, sizeof(_pinTag), "PIN/%s%d", (pin & IOT_PIN_VIRTUAL ? "V" : strNull), pin & ~IOT_PIN_VIRTUAL);
        _Properties[0] = this;
    }

    bool pinFell(void) { return !(_dbState & _BV(PIN_STATE_DEBOUNCED)) && (_dbState & _BV(PIN_STATE_CHANGED)); }
    bool pinRose(void) { return (_dbState & _BV(PIN_STATE_DEBOUNCED)) && (_dbState & _BV(PIN_STATE_CHANGED)); }

    bool pinState(void) { return _pinState; }

    void pinState(bool ps, bool urgent = false)
    {
        if (_state == IOT_RUNNING && (_pinMode & OUTPUT) && !(_dataFlags & IOT_FLAG_READONLY)) {
            if (ps != _pinState) {
                _pinState = ps;

                ESP_LOGD(_tag, "Set State: %d, urgent: %d", _pinState, urgent);
                
                if (_propUpdate(this))
                    _postUpdate(this, urgent);
            }
        }
    }

    uint8_t getMode(void) { return _pinMode; }
    
    uint8_t setMode(uint8_t mode)
    {
        if (_pin & IOT_PIN_VIRTUAL) {
            _dataFlags &= 0xFFF0;
            _dataFlags |= (IOT_FLAG_CONTROL|IOT_FLAG_SENSOR);     
            return (_pinMode = mode);
        }else if (!digitalPinIsValid(_pin))
            return -1;
        
        ESP_LOGI(_tag, "Setting Mode: 0x%X", mode);
        
        // Input/Output Mode
        if (mode & 0x0F) {
            _dataFlags &= 0xFFF0;

            if (mode & OUTPUT)
                _dataFlags |= IOT_FLAG_CONTROL;    

            if (mode & INPUT) {
                _dataFlags |= IOT_FLAG_SENSOR;
                if (!mode & OUTPUT)    
                    _dataFlags |= IOT_FLAG_READONLY;
            }
            
        }else if (mode & OPEN_DRAIN) {
            // Set to unknown
            _dataFlags &= (0xFFF0 | IOT_FLAG_READONLY);
        }

        _pinMode = mode;
        pinMode(_pin, _pinMode);

        return _pinMode; 
    }

    void setDebounce(uint32_t interval, PIN_DEBOUNCE dbMode)
    {
        if (!(_pinMode & INPUT))
            _dbMode = PIN_DEBOUNCE::OFF;
        
        timerReset();
        timerPeriod(interval);
        if ((_dbMode = interval != 0 ? dbMode : PIN_DEBOUNCE::OFF) == PIN_DEBOUNCE::LOCKOUT)
            startMillis = 0;     
    }

  protected:
    void iotStartup(void)
    {
        if (_state != IOT_STOPPED)
            return;

        _dbState = 0;
            
        if (digitalPinIsValid(_pin)) {
            setMode(_pinMode);

            if (_pinMode & OUTPUT)
                _pinWrite();

            if ((_pinState = _pinRead()))
                _dbState = _BV(PIN_STATE_DEBOUNCED) | _BV(PIN_STATE_UNSTABLE);
            
            startMillis = _dbMode != PIN_DEBOUNCE::LOCKOUT ? millis() : 0;
            _state = IOT_RUNNING;
            return;            
        }else if(_pin & IOT_PIN_VIRTUAL) {
            timerPeriod(0);
            _state = IOT_RUNNING;
            return;                        
        }

        ESP_LOGE(_tag, "Invalid Digital Pin");
        _state = IOT_ERROR;
    }

    void iotShutdown(void)
    {
        if (_state == IOT_RUNNING && !(_pin & IOT_PIN_VIRTUAL))
            pinMode(_pin, _safeMode);
        _state = IOT_STOPPED;    
    }

    void iotService(void)
    {
        if (_state != IOT_RUNNING || (_pin & IOT_PIN_VIRTUAL))
            return;
        
        if (_pinMode & INPUT) {
            if (_dbMode == PIN_DEBOUNCE::LOCKOUT) {
                _dbState &= ~_BV(PIN_STATE_CHANGED);

                // Ignore everything if we are locked out
                if (timerExpired()) {
                    bool currentState = _pinRead();

                    if ((bool)(_dbState & _BV(PIN_STATE_DEBOUNCED)) != currentState) {
                        timerReset();
                        _dbState ^= _BV(PIN_STATE_DEBOUNCED);
                        _dbState |= _BV(PIN_STATE_CHANGED);
                    }
                }
            }else if(_dbMode == PIN_DEBOUNCE::PROMPT) {
                bool readState = _pinRead();
                _dbState &= ~_BV(PIN_STATE_CHANGED);
                
                if (readState != (bool)(_dbState & _BV(PIN_STATE_DEBOUNCED))) {
                    // We have seen a change from the current button state.
                    if (timerExpired()) {
                        // We have passed the time threshold, so a new change of state is allowed.
                        // set the STATE_CHANGED flag and the new DEBOUNCED_STATE.
                        // This will be prompt as long as there has been greater than interval ms since last change of input.
                        // Otherwise debounced state will not change again until bouncing is stable for the timeout period.
                        _dbState ^= _BV(PIN_STATE_DEBOUNCED);
                        _dbState |= _BV(PIN_STATE_CHANGED);
                    }
                }
            
                // If the readState is different from previous readState, reset the debounce timer - as input is still unstable
                // and we want to prevent new button state changes until the previous one has remained stable for the timeout.
                if (readState != (bool)(_dbState & _BV(PIN_STATE_UNSTABLE)) ) {
                    // Update Unstable Bit to match readState
                    _dbState ^= _BV(PIN_STATE_UNSTABLE);
                    timerReset();
                }

            }else if(_dbMode == PIN_DEBOUNCE::STABLE) {
                bool currentState = _pinRead();
                _dbState &= ~_BV(PIN_STATE_CHANGED);

                // If the reading is different from last reading, reset the debounce counter
                if (currentState != (bool)(_dbState & _BV(PIN_STATE_UNSTABLE)) ) {
                    _dbState ^= _BV(PIN_STATE_UNSTABLE);
                    timerReset();
                }else if (timerExpired()) {
                    // We have passed the threshold time, so the input is now stable
                    // If it is different from last state, set the STATE_CHANGED flag
                    if ((bool)(_dbState & _BV(PIN_STATE_DEBOUNCED)) != currentState) {
                        timerReset();
                        _dbState ^= _BV(PIN_STATE_DEBOUNCED);
                        _dbState |= _BV(PIN_STATE_CHANGED);
                    }
                }
            }else {
                _pinState = _pinRead();
                _dbState &= ~_BV(PIN_STATE_CHANGED);
                if ((bool)(_dbState & _BV(PIN_STATE_DEBOUNCED)) != _pinState) {
                    _dbState ^= _BV(PIN_STATE_DEBOUNCED);
                    _dbState |= _BV(PIN_STATE_CHANGED);
                }
            }

            // If state has changed, post update
            _pinState & _BV(PIN_STATE_DEBOUNCED);
            if (_dbState & _BV(PIN_STATE_CHANGED))
                _postUpdate(this);
        }
    }

    bool _propUpdate(IOTProperty *prop)
    {
        if (prop == this && _pinMode & OUTPUT) {
            ESP_LOGD(_tag, "State Changed: %d", _pinState);
            
            if (! (_pin & IOT_PIN_VIRTUAL))
                _pinWrite();
            return true;
        }
        return false;
    }

    char _pinTag[16];
    bool _pinState;
    uint8_t _pin, _pinMode, _safeMode;

private:
    uint8_t _pinRead(void)
    {
        if (_flags & IOT_FLAG_INVERT)
            return !digitalRead(_pin);
        return digitalRead(_pin);
    }

    void _pinWrite()
    {
        if (_flags & IOT_FLAG_INVERT)
            digitalWrite(_pin, !_pinState);        
        digitalWrite(_pin, _pinState);
    }

    uint8_t _dbState;
    PIN_DEBOUNCE _dbMode;
};

#endif // _IOT_PIN_H
/******************************************************************************/
