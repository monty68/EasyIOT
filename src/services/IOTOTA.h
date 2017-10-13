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
#ifndef _IOTOTA_H
#define _IOTOTA_H

#include "core/IOTFunction.h"
#include "core/IOTHttp.h"

/*
** "Over the Air" Update Function Class
*/
class IOTOTA : public IOTFunction
{
public:
  IOTOTA(uint16_t port = 3232, bool auth = false);

protected:
  void iotStartup(void);
  void iotShutdown(void);
  void iotService(void);
};

#endif // _IOTOTA_H

/******************************************************************************/