/*
** EasyIOT - (OTA) "Over the Air" Update Function Class
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
#include "iotOTA.h"

/*
** Class Construction
*/
IOTOTA::IOTOTA(uint16_t port, bool auth) : IOTFunction("OTA", 0)
{
    _flags = IOT_FLAG_SYSTEM | IOT_FLAG_LOCK_LABEL;
}

void IOTOTA::iotStartup(void)
{
    if (_state != IOT_STOPPED)
        return;
    MDNS.enableArduino();
    _state = IOT_RUNNING;
}

void IOTOTA::iotShutdown(void)
{
    if (_state != IOT_RUNNING)
        return;
    MDNS.disableArduino();
    _state != IOT_STOPPED;
}

void IOTOTA::iotService(void)
{
    if (_state != IOT_RUNNING)
        return;
    yield();
}
/******************************************************************************/