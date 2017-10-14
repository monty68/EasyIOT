/*
** EasyIOT - (SSDP) Simple Service Discovery Protocol Device Class
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
#include "services/IOTSSDP.h"
#include "core/IOTRandom.h"

/*
** General Defintions and Equates
*/
#define UPNP_PCLASS PROPERTY_CLASS::GENERIC

static const char *defDeviceURL = "https://github.com/monty68/EasyIOT";
static const char *defSchemaURL = "/schema.xml";
static const char *defPresentationURL = "/index.html";
static const char *upnp_rootdevice = "upnp:rootdevice";
static const char *headerSOAPAction = "SOAPAction";
static const char *headerSID = "SID";

/*
#define UPNP_DEVICE_TYPE "urn:EasyIOT:device:esp:1"
#define UPNP_SEARCH_TYPE "urn:EasyIOT:device:**"
#define UPNP_UUID_HEADER strNull
#define UPNP_UUID_SERIAL "47843fea-e025-40bd-a38c-"

https://wso2.com/library/3212/

<?xml version='1.0' encoding='UTF-8'?>
<soapenv:Envelope xmlns:soapenv="https://schemas.xmlsoap.org/soap/envelope/">
    <soapenv:Body>
        <soapenv:Fault>
            <faultcode>soapenv:Client</faultcode>
            <faultstring>org.apache.axis2.databinding.ADBException: Unexpected subelement DeploymentDocuments
            </faultstring>
            <detail/>
        </soapenv:Fault>
    </soapenv:Body>
</soapenv:Envelope>

The next element, <faultstring>, provides a short and human readable explanation of what the error is. Both these elements, faultcode and faultstring, are mandatory elements of a SOAP fault message in SOAP 1.1. In the case of SOAP 1.2, faultstring is renamed to Reason and faultcode is renamed to Code. But this code element can contain a hierarchy of fault codes. Code listing 2 shows a segment from a SOAP 1.2 fault message, with two fault codes encapsulated within the Code element.

*/

const char _upnp_schema_template[] =
    "<?xml version=\"1.0\"?>"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<URLBase>http://%s:%u/</URLBase>"
    "<device>"
    "<friendlyName>%s</friendlyName>"
    "<deviceType>%s</deviceType>"
    "<manufacturer>%s</manufacturer>"
    "<manufacturerURL>%s</manufacturerURL>"
    // <modelDescription></<modelDescription>
    "<modelName>%s</modelName>"
    "<modelNumber>%s</modelNumber>"
    "<modelURL>%s</modelURL>"
    "<serialNumber>%s</serialNumber>"
    "<UDN>uuid:%s</UDN>"
    // <UPC></UPC>
    "<serviceList>%s</serviceList>"
    "<deviceList>%s</deviceList>"
    "<presentationURL>%s</presentationURL>"
    "</device>"
    "<specVersion>"
    "<major>1</major>"
    "<minor>0</minor>"
    "</specVersion>"
    "</root>\r\n"
    "\r\n";

/*
** Class Construction
*/
UPNPDevice::UPNPDevice(const char *deviceType, uint16_t port, uint32_t interval)
    : IOTPropertyString(this, 0, nullptr, UPNP_UUID_SIZE, UPNP_PCLASS, strNull, strNull, deviceType),
      IOTFunction(_devTag, 9),
      IOTTimer(interval * 1000L),
      _webPort(port),
      _pending(false),
      _isRoot(false),
      _interval(interval),
      _sdpServer(nullptr),
      _webServer(nullptr),
      _ssdpHeader(nullptr),
      _nextDevice(nullptr)
{
    char buf[200] = {0};
    uint16_t pFlags = IOT_FLAG_SYSTEM | IOT_FLAG_CONFIG;
    _dataFlags = IOT_FLAG_SYSTEM | IOT_FLAG_CONFIG | IOT_FLAG_READONLY | IOT_FLAG_LOCK_LABEL;
    _flags = IOT_FLAG_SYSTEM | IOT_FLAG_CONFIG | IOT_FLAG_VOLATILE;
    
    snprintf(_devTag, sizeof(_devTag) - 1, "SSDP/%d", port);
    _dataLabel = (char *)deviceType;
    _Properties[0] = this; // UUID
    
    // Schema URL
    _Properties[1] = new IOTPropertyString(this, pFlags, defSchemaURL, UPNP_SCHEMA_URL_SIZE, UPNP_PCLASS);

    // Serial Number
    snprintf(buf, UPNP_SERIAL_NUMBER_SIZE, "%" PRIu64, ESP.getEfuseMac());
    _Properties[2] = new IOTPropertyString(this, pFlags, buf, UPNP_SERIAL_NUMBER_SIZE, UPNP_PCLASS);

    // Model Name
    snprintf(buf, UPNP_MODEL_NAME_SIZE, "%s:%s", strEasyIoT, IOTMaster::iotModel());
    _Properties[3] = new IOTPropertyString(this, pFlags, buf, UPNP_MODEL_NAME_SIZE, UPNP_PCLASS);

    // Model Number
    snprintf(buf, UPNP_MODEL_VERSION_SIZE, "%s:%s", IOTMaster::iotVersion(), IOTMaster::iotBoard());
    _Properties[4] = new IOTPropertyString(this, pFlags, buf, UPNP_MODEL_VERSION_SIZE, UPNP_PCLASS);

    // Model URL
    _Properties[5] = new IOTPropertyString(this, pFlags, defDeviceURL, UPNP_MODEL_URL_SIZE, UPNP_PCLASS);

    // Manufacturer
    snprintf(buf, UPNP_MANUFACTURER_SIZE, "%s", strEasyIoT);
    _Properties[6] = new IOTPropertyString(this, pFlags, buf, UPNP_MANUFACTURER_SIZE, UPNP_PCLASS);

    // Manufacturer URL
    _Properties[7] = new IOTPropertyString(this, pFlags, defDeviceURL, UPNP_MANUFACTURER_URL_SIZE, UPNP_PCLASS);
}

/*
** Class Destruction
*/
UPNPDevice::~UPNPDevice()
{
    // Make sure we are all closed down
    iotShutdown();
    if (_webServer != NULL && _webOwner)
        delete _webServer;
}

/*
** Service Startup
*/
void UPNPDevice::iotStartup(void)
{
    if (_state != IOT_STOPPED)
        return;

    if (_iotMaster == nullptr)
    {
        _state = IOT_ERROR;
        return;
    }

    const char *uuidHeader = (_dataPrefix != NULL) ? _dataPrefix : strNull;

    if ((_dataVal != nullptr) && *_dataVal == '\0')
    {
        String uuid = iotRandom.uuidGenerator(true);
        ESP_LOGI(_tag, "Generated UUID - (%s)%s", uuidHeader, uuid.c_str());
        _dataFlags &= ~IOT_FLAG_READONLY;
        snprintf(_dataVal, UPNP_UUID_SIZE, "%s%s", uuidHeader, uuid.c_str());
        _saveProperty(this);
    }
    _dataFlags |= IOT_FLAG_READONLY;

    // Setup web service
    if (_webServer == nullptr)
    {
        if (_iotMaster->Server() != nullptr)
        {
            if (_iotMaster->Server()->webPort() == _webPort)
                _webServer = _iotMaster->Server();
        }

        if (_webServer == nullptr)
        {
            _webOwner = true;
            _webServer = new IOTHTTP(_tag, _webPort, false);
            _webServer->webStartup();
        }

        if (_webServer == nullptr)
        {
            _state = IOT_ERROR;
            return;
        }
        else
        {
            const char * headerkeys[] = { headerSOAPAction, headerSID } ;
            size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);

            //ask server to track these headers
            _webServer->collectHeaders(headerkeys, headerkeyssize);
            _webServer->webHandler(this);
            
            //_webServer->on(upnpSchemaURL(), HTTP_GET, std::bind(&UPNPDevice::sendSchema, this, std::placeholders::_1));
        }
    }

    if (_sdpServer == nullptr)
    {
        _state = IOT_ERROR;
        return;
    }

    _sdpServer->iotNotify(this, SSDP::ALIVE);
    _pending = false;
    _state = IOT_RUNNING;
}

/*
** Service Shutdown
*/
void UPNPDevice::iotShutdown(void)
{
    if (_state == IOT_RUNNING)
    {
        if (_sdpServer != nullptr)
            _sdpServer->iotNotify(this, SSDP::BYEBYE);

        // If we own a HTTP Server Service, stop it!
        if (_webServer != nullptr && _webOwner)
        {
            _webServer->webShutdown();
            delete _webServer;
            _webServer = nullptr;
            _webOwner = false;
        }
    }
    _state = IOT_STOPPED;
}

/*
** Service Update
*/
void UPNPDevice::iotService(void)
{
    if (_state != IOT_RUNNING)
        return;

    // If we own a HTTP Server, service it!
    if (_webServer != nullptr && _webOwner)
        _webServer->webService();

    // SSDP Keepalive
    if (timerExpired())
    {
        _sdpServer->iotNotify(this, SSDP::ALIVE);
        timerReset();
    }
    else
        yield();
}

bool UPNPDevice::_propUpdate(IOTProperty *prop)
{
    // Any property changes, stop and restart
    _flags |= IOT_FLAG_RESTART;
    return true;
}

/*
** Send Notification Broadcast
*/
void UPNPDevice::iotNotify(ssdp_method_t method)
{
    if (_sdpServer != nullptr)
    {
        _sdpServer->iotNotify(this, method);
        timerReset();
    }
}

/*
** Compare DeviceType with passed query string
*/
bool UPNPDevice::upnpCanHandle(String &st)
{
    if (st.equalsIgnoreCase(upnp_rootdevice))
    {
        ESP_LOGV(_tag, "[%s]=%d - %s", st.c_str(), _isRoot, upnpDeviceType().c_str());
        return _isRoot;
    }

    if (st.equalsIgnoreCase(upnpDeviceType()) || st.equalsIgnoreCase(upnpSearchType()))
    {
        ESP_LOGV(_tag, "[%s] - %s", st.c_str(), upnpDeviceType().c_str());
        return true;
    }

    return false;
}

bool UPNPDevice::httpCanHandle(HTTPMethod method, String uri)
{
    if (uri == defPresentationURL)
        return true;
    if (method == HTTP_GET && uri == upnpSchemaURL())
        return true;
    //if (method == HTTP_GET && uri == upnpServiceURL())
        //return true;
    if (uri.startsWith("/upnp/"))
        return true;
    return false;
}

bool UPNPDevice::httpHandle(IOTHTTP &server, HTTPMethod method, String uri)
{
    if (_state != IOT_RUNNING)
        server.send(503);

    // Presentation URL, send redirect to masters web server port/page
    //
    if (uri == "/" || uri == defPresentationURL)
    {
        uint16_t pPort = Master()->Server()->webPort();

        if (pPort != _webPort) {
            server.sendHeader("Location", upnpPresentation(server));
            // UPNP 1.1 Spec: Send 307 NOT 301
            return server.send(307);
        }

        return server.send(503);
    }

    // Send Device Schema
    //
    if (method == HTTP_GET && uri == upnpSchemaURL())
    {
        char buffer[1460];
        
        snprintf(buffer, sizeof(buffer), _upnp_schema_template,
                 WiFi.localIP().toString().c_str(), _webPort,
                 getLabel(),
                 upnpDeviceType().c_str(),
                 upnpManufacturer().c_str(),
                 upnpManufacturerURL().c_str(),
                 upnpModelName().c_str(),
                 upnpModelNumber().c_str(),
                 upnpModelURL().c_str(),
                 upnpSerialNumber().c_str(),
                 upnpUUID().c_str(),
                 upnpServiceList(server).c_str(),
                 upnpDeviceList(server).c_str(),
                 upnpPresentation(server).c_str());

        return server.send(200, MIME_TYPE_XML, buffer);
    }

    // SOAP Action(s)
    //
    // The presence and content of the SOAPAction header field can be used by servers such as 
    // firewalls to appropriately filter SOAP request messages in HTTP. The header field value 
    // of empty string ("") means that the intent of the SOAP message is provided by the HTTP 
    // Request-URI. No value means that there is no indication of the intent of the message.
    //    
    String urn("");
    String act("");

    if (server.hasHeader(headerSOAPAction)) {
        urn = server.header(headerSOAPAction);

        ESP_LOGD(_tag, "SOAPAction: %s (%d, %s)", urn.c_str(), method, uri.c_str());
        if (urn == "")
            return server.send(500);

        if (urn[0] == '"')
            urn = urn.substring(1, urn.length() - 1);

        int hash = urn.indexOf('#');

        if (hash) {
            act = urn.substring(hash + 1);
            urn.remove(hash);
        }

        ESP_LOGD(_tag, "URN: %s", urn.c_str());
        ESP_LOGD(_tag, "ACT: %s", act.c_str());
        
        return true;    
    }

    ESP_LOGD(_tag, "Handle: %d, %s", method, uri.c_str());    
    
    return false;
}

String UPNPDevice::upnpPresentation(IOTHTTP &server)
{
    uint16_t pPort = Master()->Server()->webPort();
    const char *path = defPresentationURL;

    if (*path == '/')
        path++;

    if (pPort != _webPort)
    {
        char url[200];

        sprintf(url, "http://%s:%d/%s", WiFi.localIP().toString().c_str(), pPort, path);
        return String(url);
    }
    return String(path);
}

String UPNPDevice::upnpServiceList(IOTHTTP &server)
{
    return String();
}

String UPNPDevice::upnpDeviceList(IOTHTTP &server)
{
    return String();
}

/******************************************************************************/