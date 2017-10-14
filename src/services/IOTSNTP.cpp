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
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "WiFi.h"
#include <esp32-hal.h>
#include <apps/sntp/sntp.h>
#include "IOTSNTP.h"

#define IOTSNTP_PROPERTIES 2
#define IOTSNTP_MAX_PROPERTIES (IOTSNTP_PROPERTIES + SNTP_MAX_SERVERS)

/*
** Class Construction
*/
IOTSNTP::IOTSNTP(const char *defTZ)
    : IOTFunction("SNTP", IOTSNTP_MAX_PROPERTIES),
      IOTProperty(this, IOT_FLAG_READONLY, sizeof(_timeStr), PROPERTY_TYPE::DATE, PROPERTY_CLASS::TIME, strTime, NULL, strSystem),
      _timeTick(0)
{
    _timeInfo = {0};

    _flags = IOT_FLAG_SYSTEM | IOT_FLAG_CONFIG | IOT_FLAG_LOCK_LABEL;
    _dataFlags = _flags | IOT_FLAG_READONLY  | IOT_FLAG_VOLATILE;
    _Properties[0] = this;
    _Properties[1] = new IOTPropertyString(this, _flags, defTZ, 28, PROPERTY_CLASS::TIMEZONE);

    for (int h = 0; h < SNTP_MAX_SERVERS; h++)
    {
        _Properties[IOTSNTP_PROPERTIES + h] = new IOTPropertyString(this, _flags, "pool.ntp.org", IOT_MAX_HOST, PROPERTY_CLASS::IPHOST, strTimeServer);
    }
}

/*
** Service Startup
*/
void IOTSNTP::iotStartup(void)
{
    if (_state != IOT_STOPPED)
        return;

    ESP_LOGI(_tag, "Initializing Service (%d)", SNTP_MAX_SERVERS);

    // Just to make sure!
    if (sntp_enabled())
    {
        sntp_stop();
    }

    char buf[80];

    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    for (int h = 0; h < SNTP_MAX_SERVERS; h++)
    {
        _Properties[IOTSNTP_PROPERTIES + h]->getData(buf, sizeof(buf));
        ESP_LOGD(_tag, "Time Server #%d: %s", h + 1, buf);
        sntp_setservername(h, buf);
    }

    sntp_init();

    if (sntp_enabled())
    {
        setenv("TZ", _Properties[1]->getData(buf, sizeof(buf)), 1);
        ESP_LOGD(_tag, "Timezone: %s", buf);
        tzset();
        _state = IOT_RUNNING;

        // wait for the service to set the time
        int count = 0;
        time(&_timeTick);
        localtime_r(&_timeTick, &_timeInfo);
        
        while (_timeInfo.tm_year < (2016 - 1900) && sntp_enabled() && count < 10)
        {
            if (count++ % 3)
            {
                ESP_LOGD(_tag, "Syncing time... (forced)");
                sntp_stop();
                sntp_init();
            }else
                ESP_LOGI(_tag, "Syncing time...");
            
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            time(&_timeTick);
            localtime_r(&_timeTick, &_timeInfo);
        }

        if (sntp_enabled())
            _state = IOT_RUNNING;
        
        strftime(buf, sizeof(buf), "%c %Z", &_timeInfo);
        ESP_LOGI(_tag, "Date/time is: %s", buf);
    }
}

/*
** Service Shutdown
*/
void IOTSNTP::iotShutdown(void)
{
    if (_state == IOT_RUNNING)
    {
        if (sntp_enabled())
        {
            sntp_stop();
        }
    }
    _state = IOT_STOPPED;
}

/*
** Service Update
*/
void IOTSNTP::iotService(void)
{
    time(&_timeTick);
    localtime_r(&_timeTick, &_timeInfo);
    yield();
}

bool IOTSNTP::_propUpdate(IOTProperty *prop)
{
    // Any property changes, we restart the service
    _flags |= IOT_FLAG_RESTART;
    return true;
}

String IOTSNTP::getData(void)
{
    time(&_timeTick);
    localtime_r(&_timeTick, &_timeInfo);
    strftime(_timeStr, sizeof(_timeStr), "%c %Z", &_timeInfo);

    return String(_timeStr);
}

bool IOTSNTP::_dataSet(String &newVal)
{
    return true;
}
/******************************************************************************/