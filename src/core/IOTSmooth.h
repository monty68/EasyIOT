/*
** EasyIOT - Generic Signal Smoothing Class
**
** Class based on code created 22 Apr 2007 by David A. Mellis  <dam@mellis.org>
** and later modified 9 Apr 2012 by Tom Igoe
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
#ifndef _IOT_SMOOTH_H
#define _IOT_SMOOTH_H

#include <Arduino.h>

template <class _T>
class IOTSmooth
{
  public:
    IOTSmooth(uint16_t numReadings = 10) : _nxtReading(0)
    {
        if (numReadings > 100)
        {
            numReadings = 100;
        }
        if (numReadings < 1)
        {
            numReadings = 1;
        }
        _numReadings = numReadings;

        _rawReadings = new _T[_numReadings];
        memset(_rawReadings, 0, sizeof(_T) * _numReadings);
    }

    ~IOTSmooth()
    {
        delete _rawReadings;
    }

    _T smooth(_T value)
    {
        if (_numReadings <= 1)
            return value;
        _rawReadings[_nxtReading++] = value;

        _T total = 0;
        for (int r = 0; r < _nxtReading; r++)
            total += _rawReadings[r];
        total /= _nxtReading > 0 ? _nxtReading : 1;

        if (_nxtReading >= _numReadings)
            _nxtReading = 0;

        return total;
    }

  protected:
    uint16_t _nxtReading;
    uint16_t _numReadings;
    _T *_rawReadings;
};

#endif // _IOT_SMOOTH_H
/******************************************************************************/
