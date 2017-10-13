/*
** EasyIOT - (SSDP) Simple Service Discovery Protocol Function Class
**
** This was inspired from the ESP8266SSDP library, Copyright (c) 2015 Hristo Gochkov
** Original (Arduino) version by Filippo Sallemi, July 23, 2014.
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
#ifndef _IOT_SSDP_H
#define _IOT_SSDP_H

#include "core/IOTFunction.h"
#include "core/IOTTimer.h"
#include "core/IOTHttp.h"

/*
** Methods
*/
typedef enum class SSDP {
  NONE,
  ALIVE,
  BYEBYE,
  UPDATE
} ssdp_method_t;

/*
** Forward Reference
*/
class IOTSSDP;
#include "ssdp/UPNPDevice.h"

/*
** SSDP Function Class
*/
class IOTSSDP : public IOTFunction, private IOTTimer
{
public:
  friend class UPNPDevice;
  IOTSSDP(uint8_t ttl = 0);
  ~IOTSSDP();
  void addDevice(UPNPDevice& device) { addDevice(&device); }  
  void addDevice(UPNPDevice *device);
  
protected:
  void iotStartup(void);
  void iotShutdown(void);
  void iotService(void);
  void iotNotify(UPNPDevice *device, ssdp_method_t method);

private:
  void _respond(UPNPDevice *device, ssdp_method_t method);  
  bool _parsePacket(void);  
  int _parseToken(String *token, bool break_on_space, bool break_on_colon);
  void _bailRead(void);

  WiFiUDP _udpServer;  
  UPNPDevice *_firstDevice;
  UPNPDevice *_lastDevice;
  uint8_t _ttl;
  bool _pending;
  
  IPAddress _pendingAddr;
  uint16_t _pendingPort;
  String _st;
  int _mx;
};

#endif // _IOTSSDP_H
/******************************************************************************/
