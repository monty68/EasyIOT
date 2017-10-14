/*
** EasyIOT - (NTP/RTC) Time Function Class
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
#ifndef _IOT_SNTP_H
#define _IOT_SNTP_H

#include "core/IOTFunction.h"

#define IOSNTP_MAX_STRING       30

class IOTSNTP : public IOTFunction, public IOTProperty
{
  public:
    IOTSNTP(const char *defTZ = "UCT");
    time_t timeTick(void) const { return _timeTick; }
    struct tm *tickInfo(void) { return &_timeInfo; }
    String getData(void);
    
  protected:
    void iotStartup(void);
    void iotShutdown(void);
    void iotService(void);
    bool _propUpdate(IOTProperty *prop);

    void * _dataPtr() { return (void *)&_timeStr[0]; }
    bool _dataSet(String& newVal);

  private:
    time_t _timeTick;
    struct tm _timeInfo;
    char _timeStr[IOSNTP_MAX_STRING];
};

#endif // _IOT_SNTP_H
/******************************************************************************/
