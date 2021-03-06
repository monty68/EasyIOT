/*
** EasyIOT - Main Device Class
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
#if defined(ARDUINO_ARCH_ESP32)
#if !defined(_IOT_H)
#define _IOT_H

#include <Arduino.h>
#include <esp_log.h>
#include "core/IOTProperty.h"
#include "core/IOTFunction.h"
#include "core/IOTPIN.h"

#include "services/IOTSNTP.h"
#include "services/IOTSSDP.h"
#include "services/IOTOTA.h"

#endif // _IOT_H
#else // ARDUINO_ARCH_ESP32
  #error "EasyIOT (currently) only supports ESP32 boards!"
#endif
/******************************************************************************/
