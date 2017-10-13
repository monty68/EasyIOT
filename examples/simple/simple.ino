/*
** EasyIOT - Examples: simple.ino, Emulate a Belkin WeMo Switch/Outlet
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
#include <EasyIOT.h>

/*
** Configuration Defintions
*/
#define EASYIOT_WIFI_SSID "myssid"
#define EASYIOT_WIFI_PASS "mypass"
#define EASYIOT_TIME_ZONE "UTC"
#define EASYIOT_HTTP_PORT 80
#define EASYIOT_SSDP_PORT EASYIOT_HTTP_PORT

/*
** Declare the IOT Master and Services
*/
IOT iot(EASYIOT_WIFI_SSID, EASYIOT_WIFI_PASS, EASYIOT_HTTP_PORT);
IOTSNTP iotSNTP(EASYIOT_TIME_ZONE);
IOTSSDP iotSSDP(EASYIOT_SSDP_PORT);

/*
** Declare the IOT Functions we want to use
*/
#include "emu/wemo/WEMOSwitch.h"
IOTPIN myRelay(27);
IOTWEMOS iotWEMO1(myRelay, EASYIOT_SSDP_PORT);

/*
** Setup Function
*/
void setup()
{
    // Have some debug messages
    esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("wifi", ESP_LOG_WARN);
  
    // Name the Device by setting and locking the masters label
    iot.setLabel("Example Thing", true);

    // Functions are started by the master
    // in the order they were added, so we
    // want the time sync to happen first
    iot.addFunction(iotSNTP);

    // Emulate a Belkin Wemo Switch/Outlet/Socket
    //
    iot.addFunction(iotSSDP);
    iotSSDP.addDevice(iotWEMO1);

    // Give the device a friendly name
    iotWEMO1.setLabel("my relay");

    // Now startup the IOT functions
    //
    iot.iotStartup();
}

/*
** Program Loop
*/
void loop()
{
    // Call the master IOT service handler
    //
    // Its important to do as little as possible
    // within this loop. Do NOT use delay() etc.
    //
    iot.iotService();
}
/******************************************************************************/