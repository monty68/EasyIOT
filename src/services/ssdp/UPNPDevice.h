/*
** EasyIOT - (SSDP) Simple Service Discovery Protocol UPNP Device Class
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
#ifndef _IOT_UPNP_DEVICE_H
#define _IOT_UPNP_DEVICE_H

/*
** General Defintions and Equates
*/
#define UPNP_UUID_HEADER_SIZE 20
#define UPNP_UUID_SIZE (UPNP_UUID_HEADER_SIZE + IOT_UUID_LENGTH)
#define UPNP_SCHEMA_URL_SIZE 64
#define UPNP_DEVICE_TYPE_SIZE 64
//#define UPNP_FRIENDLY_NAME_SIZE 64
#define UPNP_SERIAL_NUMBER_SIZE 32
#define UPNP_PRESENTATION_URL_SIZE 128
#define UPNP_MODEL_NAME_SIZE 64
#define UPNP_MODEL_URL_SIZE 128
#define UPNP_MODEL_VERSION_SIZE 32
#define UPNP_MANUFACTURER_SIZE 64
#define UPNP_MANUFACTURER_URL_SIZE 128

#define SSDP_NOTIFY_INTERVAL 1200
#define SSDP_SAFE_PORT_MIN 49500

/*
** Device Class
*/
class UPNPDevice : public IOTFunction, public IOTPropertyString, private IOTTimer
{
    public:
        friend class IOTSSDP;
        UPNPDevice(const char *deviceType, uint16_t port = 9980, uint32_t interval = SSDP_NOTIFY_INTERVAL);
        ~UPNPDevice();  
        IOTHTTP *Server(void) { return _webServer; }
        void iotNotify(ssdp_method_t method = SSDP::UPDATE);

        uint32_t upnpInterval(void) { return _interval; }
        uint16_t upnpPort(void) { return _webPort; }
        bool upnpRootDevice(void) { return _isRoot; }        
        String upnpUUID(void) const { return String(_dataVal); }
        String upnpUUIDHeader(void) const { return String(_dataPrefix); }
        String upnpSearchType(void) const { return String(_dataSuffix); }
        String upnpDeviceType(void) const { return String(_dataLabel); }

        String upnpSchemaURL(void) { return Property(1)->getData(); }
        String upnpSerialNumber(void) { return Property(2)->getData(); }        
        String upnpModelName(void) { return Property(3)->getData(); }
        String upnpModelNumber(void) { return Property(4)->getData(); }
        String upnpModelURL(void) { return Property(5)->getData(); }
        String upnpManufacturer(void) { return Property(6)->getData(); }
        String upnpManufacturerURL(void) { return Property(7)->getData(); }
        String upnpPresentationURL(void) { return Property(8)->getData(); }

        bool upnpSchemaURL(String& s) { return Property(1)->setData(s); }
        bool upnpSerialNumber(String& s) { return Property(2)->setData(s); }        
        bool upnpModelName(String& s) { return Property(3)->setData(s); }
        bool upnpModelNumber(String& s) { return Property(4)->setData(s); }
        bool upnpModelURL(String& s) { return Property(5)->setData(s); }
        bool upnpManufacturer(String& s) { return Property(6)->setData(s); }
        bool upnpManufacturerURL(String& s) { return Property(7)->setData(s); }
        bool upnpPresentationURL(String& s) { return Property(8)->setData(s); }
        
    protected:
        virtual void iotStartup(void);
        virtual void iotShutdown(void);
        virtual void iotService(void);
        virtual bool _propUpdate(IOTProperty *prop);        
        virtual void sendSchema(IOTHTTP &server);
        virtual String schemaExtra(IOTHTTP &server);
        virtual bool testDevice(String& st);


        void soapSend();
        void soapError();


        UPNPDevice *nextDevice() { return _nextDevice; }
        void nextDevice(UPNPDevice *d) { _nextDevice = d; }

        IOTSSDP *_sdpServer;
        IOTHTTP *_webServer;
        uint16_t _webPort;
        bool _webOwner;
        bool _pending;
        bool _isRoot;
        const char * _ssdpHeader;
        char _devTag[15];
        uint32_t _interval;        
      
    private:        
        UPNPDevice *_nextDevice = nullptr;
};

#endif // _IOT_UPNP_DEVICE_H
/******************************************************************************/