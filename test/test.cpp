#define EASYIOT_WIFI_SSID "VM1132554"
#define EASYIOT_WIFI_PASS "4wtgyyjVkvts"
#define EASYIOT_TIME_ZONE "GMT0BST,M3.5.0/1,M10.5.0"

/* GIT Cleanup
  Delete .git folder, then in git bash:

    git init
    git remote add origin git@github.com:user/repo
    git add '.travis.yml'
    git add '.gitignore'
    git add *
    git commit -am 'message'
    git push -f origin master
*/

#include "EasyIOT.h"
#include "i2c/BH1750.h"

#define FLASH_PIN 0
#define LED_BUILTIN 2

IOT iot(EASYIOT_WIFI_SSID, EASYIOT_WIFI_PASS, 8080);
IOTSNTP iotSNTP(EASYIOT_TIME_ZONE);
IOTSSDP iotSSDP;
IOTOTA iotOTA;

IOTPIN pwrRelay(12, OUTPUT, PIN_CLASS::RELAY);
IOTPIN pirSensor(27, INPUT, PIN_CLASS::MOTION);

BH1750 luxSensor(I2C_ADDRESS_BH1750);
//INA3221 pwrSensor(I2C_ADDRESS_INA3221_0);

#include "emu/wemo/WEMOSwitch.h"
IOTWEMOS iotWEMO1(pwrRelay, 8080);
//IOTWEMOS iotWEMO2(pirSensor, 9982);

/*
** Program Setup
*/
void setup()
{
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  esp_log_level_set("wifi", ESP_LOG_WARN);
  esp_log_level_set("SSDP", ESP_LOG_DEBUG);
  
  Serial.begin(115200);

  iot.setLabel("Environment and Lighting Bot", false);
  iot.addFunction(iotSNTP);
  //iot.addFunction(iotOTA);

  /*
  ** Emulate a Belkin Wemo Socket/Switch/Outlet
  */
  iot.addFunction(iotSSDP);
  iotSSDP.addDevice(iotWEMO1);  
  iotWEMO1.setLabel("porch");

  //iotSSDP.addDevice(iotWEMO2).setLabel("porch light");
  //pirSensor.setLabel("Movement Sensor", true);
  //iot.addFunction(pirSensor);

  //pwrRelay.setLabel("Power Relay", true);
  //iot.addFunction(pwrRelay);

  /*
  pwrSensor.setLabel("Lighting Supply Monitor", true);
  pwrSensor.Property(0)->setLabel("DMX 1-8", true);
  pwrSensor.Property(1)->setLabel("DMX 9-16", true);
  pwrSensor.Property(2)->setLabel("Pixel String", true);
  iot.addFunction(pwrSensor);
  */

  //luxSensor.setLabel("Outside", true);
  //luxSensor.Property(0)->setLabel("House Front", true);
  //luxSensor.Property(0)->onChange();
  //luxSensor.Property(1)->setValue(0);
  //iot.addFunction(luxSensor);

  // We have to call Wire.begin()
  //Wire.begin();

  // Now startup all our services
  iot.iotStartup();

  /*
  String label = iot.getLabel();
  Serial.println(label);

  Serial.println("Date/Time");  
  Serial.println(iotSNTP.getData());
  */
}

/*
** Program Loop
*/
void loop()
{
  iot.iotService();
}
/*************************************************************************************/