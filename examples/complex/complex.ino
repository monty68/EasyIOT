/*
** EasyIOT - Examples: complex.ino
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
#include <i2c/INA3221.h>
#include <i2c/BH1750.h>

/*
** Configuration Defintions
*/
#define EASYIOT_WIFI_SSID "myssid"
#define EASYIOT_WIFI_PASS "mypass"
#define EASYIOT_TIME_ZONE "UTC"
#define EASYIOT_HTTP_PORT 80
#define EASYIOT_SSDP_PORT 9980

#define PIN_POWER_RELAY 12
#define PIN_MOTION_SENSOR 27

/*
** Declare the IOT Master
*/
IOT iot(EASYIOT_WIFI_SSID, EASYIOT_WIFI_PASS, EASYIOT_HTTP_PORT);

/*
** Declare the IOT Services we want to use
*/
IOTSNTP iotSNTP(EASYIOT_TIME_ZONE);
IOTSSDP iotSSDP(EASYIOT_SSDP_PORT);
IOTOTA iotOTA;

/*
** Declare the IOT Functions we want to use
*/
#include "emu/wemo/wemoSwitch.h"
BH1750 luxSensor(I2C_ADDRESS_BH1750);
INA3221 pwrSensor(I2C_ADDRESS_INA3221_0);
IOTPIN pwrRelay(PIN_POWER_RELAY);
IOTPIN pirSensor(PIN_MOTION_SENSOR);
IOTWEMO iotWEMO1(pwrRelay, EASYIOT_SSDP_PORT);

void setup()
{
  // Let's have some debug messages to see whats going on
  esp_log_level_set("*", ESP_LOG_DEBUG);
  esp_log_level_set("wifi", ESP_LOG_WARN);
  Serial.begin(115200);

  // Name the Device by setting and locking the masters label
  iot.setLabel("Environment and Lighting Bot", false);

  // Add the functions to the master (order is important!)
  //
  iot.addFunction(iotSNTP); // Sync time before we do anything else
  iot.addFunction(iotOTA);  // Over the Air Updates Enabled
  iot.addFunction(iotSSDP); // Simple Service Discovery Protocol Enabled

  // Emulate a Belkin Wemo Switch/Outlet/Socket
  //
  iot.addFunction(iotSSDP);
  iotSSDP.addDevice(iotWEMO1);

  // Give the device a friendly name
  iotWEMO1.setLabel("my relay");

  // Add Motion Sensor Function
  pirSensor.setLabel("Movement Sensor", true);
  iot.addFunction(pirSensor);

  // Add Power Relay
  pwrRelay.setLabel("Power Relay", true);
  iot.addFunction(pwrRelay);

  // Add Power Sensor (i2C device)
  pwrSensor.setLabel("Lighting Supply Monitor", true);
  pwrSensor.Property(0)->setDataLabel("DMX 1-8", true);
  pwrSensor.Property(1)->setDataLabel("DMX 9-16", true);
  pwrSensor.Property(2)->setDataLabel("Pixel String", true);
  iot.addFunction(pwrSensor);

  // Add Light Sensor (i2c device)
  luxSensor.setLabel("Outside", true);
  luxSensor.Property(0)->setDataLabel("House Front", true);
  //luxSensor.Property(0)->onChange();
  //luxSensor.Property(1)->setValue(0);
  iot.addFunction(luxSensor);

  // Now startup all our services (note we also have to start the I2C Wire library)
  Wire.begin();
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
/*************************************************************************************/