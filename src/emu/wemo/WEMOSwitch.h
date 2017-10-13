/*
** EasyIOT - Belkin WeMo (Switch/Outlet) Device Emulator Class
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
#ifndef _IOT_WEMO_SWITCH_H
#define _IOT_WEMO_SWITCH_H

#include "services/iotSSDP.h"
//#include "SOAPParser.h"

/*
** Defintions
*/
#define WEMO_UUID_PREFIX "Socket-1_0-"
#define WEMO_DEVICE_TYPE "urn:Belkin:device:controllee:1"
#define WEMO_SEARCH_TYPE "urn:Belkin:device:**"
#define WEMO_MANU_NAME "Belkin International Inc."
#define WEMO_URL_CTRL "/upnp/control/basicevent1"
#define WEMO_URL_EVNT "/upnp/event/basicevent1"
#define WEMO_URL_SCPD "/eventservice.xml"
#define WEMO_UPNP_ON "<BinaryState>1</BinaryState>"
#define WEMO_UPNP_OFF "<BinaryState>0</BinaryState>"

// Try to avoid conflicts!
#define WEMO_DEFAULT_PORT (SSDP_SAFE_PORT_MIN + 100)

class IOTWEMOS : public UPNPDevice
{
public:
  friend class IOTPIN;
  IOTWEMOS(IOTPIN &pin, uint16_t port = WEMO_DEFAULT_PORT) : IOTWEMOS(&pin, port) {}
  IOTWEMOS(IOTPIN *pin, uint16_t port = WEMO_DEFAULT_PORT)
      : UPNPDevice(WEMO_DEVICE_TYPE, port),
        _pinFunction(pin)
  {
    snprintf(_devTag, sizeof(_devTag) - 1, "WeMoS/%d", port);
    String manu = WEMO_MANU_NAME;
    upnpManufacturer(manu);
    _dataPrefix = WEMO_UUID_PREFIX;
    _dataSuffix = WEMO_SEARCH_TYPE;
  }

protected:
  IOTPIN *_pinFunction;

  /*
  ** Service Startup
  */
  void iotStartup(void)
  {
    if (_state != IOT_STOPPED)
      return;

    if (_pinFunction == nullptr)
    {
      ESP_LOGV(_tag, "Cannot start, no pin function assigned");
      _state = IOT_ERROR;
      return;
    }

    UPNPDevice::iotStartup();

    if (_state == IOT_RUNNING)
    {
      _webServer->on(WEMO_URL_SCPD, HTTP_GET, std::bind(&IOTWEMOS::_wemoSCPD, this, std::placeholders::_1));
      _webServer->on(WEMO_URL_CTRL, HTTP_ANY, std::bind(&IOTWEMOS::_wemoCTRL, this, std::placeholders::_1));
      _webServer->on(WEMO_URL_EVNT, HTTP_ANY, std::bind(&IOTWEMOS::_wemoEVNT, this, std::placeholders::_1));
    }
  }

  String schemaExtra(IOTHTTP &server)
  {
    String wemoExtra = (_pinFunction->pinState() ? WEMO_UPNP_ON : WEMO_UPNP_OFF);

    ESP_LOGD(_tag, "Schema State: %s", wemoExtra.c_str());

    wemoExtra += "<service>"
                    "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
                    "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
                    "<controlURL>" WEMO_URL_CTRL "</controlURL>"
                    "<eventSubURL>" WEMO_URL_EVNT "</eventSubURL>"
                    "<SCPDURL>" WEMO_URL_SCPD "</SCPDURL>"
                  "</service>";

    return wemoExtra;
  }

  void _wemoSCPD(IOTHTTP &server)
  {
    String eventservice_xml =
        "<?xml version=\"1.0\"?>"
        "<scpd xmlns=\"urn:Belkin:service-1-0\">"
        "<actionList>"
          "<action>"
            "<name>SetBinaryState</name>"
            "<argumentList>"
              "<argument>"
                "<retval/>"
                "<name>BinaryState</name>"
                "<relatedStateVariable>BinaryState</relatedStateVariable>"
                "<direction>in</direction>"
              "</argument>"
            "</argumentList>"
          "</action>"
        "</actionList>"
             "<serviceStateTable>"
              "<stateVariable sendEvents=\"yes\">"
                "<name>BinaryState</name>"
                "<dataType>Boolean</dataType>"
                "<defaultValue>0</defaultValue>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"yes\">"
                "<name>level</name>"
                "<dataType>string</dataType>"
                "<defaultValue>0</defaultValue>"
              "</stateVariable>"
            "</serviceStateTable>"
        "</scpd>\r\n"
        "\r\n";

    server.send(200, MIME_TYPE_XML, eventservice_xml);
  }

  void _wemoCTRL(IOTHTTP &server)
  {
    String request = server.arg(0);
    String response_xml;

    if (request.indexOf(WEMO_UPNP_ON) > 0)
    {
      ESP_LOGI(_tag, "Turn On");
      _pinFunction->pinState(true, true);

      response_xml =
          "<?xml version=\"1.0\"?>"
          "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
          "<s:Body>"
          "<u:SetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">"
          "<BinaryState>1</BinaryState>"
          "</u:SetBinaryStateResponse>"
          "</s:Body>"
          "</s:Envelope>\r\n"
          "\r\n";
      ESP_LOGD(_tag, "Response: %s", response_xml.c_str());
      server.send(200, MIME_TYPE_XML, response_xml);

      return;
    }

    if (request.indexOf(WEMO_UPNP_OFF) > 0)
    {
      ESP_LOGI(_tag, "Turn Off");
      _pinFunction->pinState(false, true);

      response_xml =
          "<?xml version=\"1.0\"?>"
          "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
          "<s:Body>"
          "<u:SetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">"
          "<BinaryState>0</BinaryState>"
          "</u:SetBinaryStateResponse>"
          "</s:Body>"
          "</s:Envelope>\r\n"
          "\r\n";

      ESP_LOGD(_tag, "Response: %s", response_xml.c_str());
      server.send(200, MIME_TYPE_XML, response_xml);

      return;
    }

    server.send(400);
  }

  void _wemoEVNT(IOTHTTP &server)
  {
    server.send(403);
  }
};
#endif // _IOT_WEMO_SWITCH_H
/******************************************************************************/
