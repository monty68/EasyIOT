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
#include "iotSSDP.h"

#undef min
#define min(a, b) ((a) < (b) ? (a) : (b))

/*
** General Defintions and Equates
*/
#define SSDP_METHOD_SIZE 10
#define SSDP_URI_SIZE 2
#define SSDP_BUFFER_SIZE 64
#define SSDP_MULTICAST_TTL 2
#define SSDP_MULTICAST_PORT 1900

static const IPAddress SSDP_MULTICAST_ADDR(239, 255, 255, 250);

static const char _ssdp_response_template[] =
    "HTTP/1.1 200 OK\r\n"
    "EXT:\r\n";

static const char _ssdp_notify_template[] =
    "NOTIFY * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "NTS: ssdp:%s\r\n";

static const char _ssdp_packet_template[] =
    "%s"
    "CACHE-CONTROL: max-age=%u\r\n"
    "SERVER: EasyIOT/%s UPNP/1.1 %s/%s\r\n"
    "USN: uuid:%s::%s\r\n"
    "%s: %s\r\n"
    "LOCATION: http://%s:%u/%s\r\n"
    //"OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
    //"01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
    "%s\r\n"
    "\r\n";

/*
** Class Construction
*/
IOTSSDP::IOTSSDP(uint8_t ttl)
    : IOTFunction("SSDP", 0), IOTTimer(0),
      _ttl(ttl),
      _firstDevice(nullptr),
      _lastDevice(nullptr)
{
    _flags = IOT_FLAG_SYSTEM | IOT_FLAG_LOCK_LABEL;
    _label = (char *)"SSDP (UDP) Responder Service";
}

/*
** Class De-construction
*/
IOTSSDP::~IOTSSDP()
{
    // Make sure we are all closed down
    iotShutdown();
}

/*
** Service Startup
*/
void IOTSSDP::iotStartup(void)
{
    if (_state != IOT_STOPPED)
        return;

    // TODO: How do we set the TTL on multicast?
    ESP_LOGI(_tag, "Starting %s", _label);
    if (_udpServer.beginMulticast(SSDP_MULTICAST_ADDR, SSDP_MULTICAST_PORT))
    {
        timerPeriod(0);
        timerReset();
        _state = IOT_RUNNING;
        return;
    }
    _state = IOT_ERROR;
}

/*
** Service Shutdown
*/
void IOTSSDP::iotShutdown(void)
{
    if (_state == IOT_RUNNING)
        _udpServer.stop();
    _state = IOT_STOPPED;
}

/*
** Service Update
*/
void IOTSSDP::iotService(void)
{
    bool waiting = false;
    UPNPDevice *device;

    if (_state != IOT_RUNNING)
        return;

    if ((!timerPeriod()) && _udpServer.parsePacket() > 0)
    {
        if ((waiting = _parsePacket()))
        {
            _pendingAddr = _udpServer.remoteIP();
            _pendingPort = _udpServer.remotePort();

            ESP_LOGV(_tag, "Packet: %s:%d [%s]", _pendingAddr.toString().c_str(), _pendingPort, _st.c_str());
            
            for (device = _firstDevice; device; device = device->nextDevice())
            {
                bool match = false;
                bool _skipAll = false;

                if (device->_state != IOT_RUNNING)
                    continue;

                if ((_st == "ssdp:all" && !_skipAll) || (match = device->testDevice(_st)))
                {
                    // Limit response delay to max 6 Seconds
                    // Amazon Alexa only waits a short time
                    //
                    uint32_t _delay = min(random(500, _mx * 1000L), 5000L);
                    if (!match)
                        ESP_LOGV(device->_tag, "[%s] - %s MX Delay: %d", _st.c_str(), device->upnpDeviceType().c_str(), _delay);
                    device->_pending = true;
                    timerPeriod(_delay);
                    timerReset();
                    _udpServer.flush();
                }
            }
        }
    }
    
    if ((timerPeriod()) && timerExpired() || _udpServer.available())
    {
        for (device = _firstDevice; device; device = device->nextDevice())
        {
            if (device->_state != IOT_RUNNING)
                continue;

            if (device->_pending)
            {
                _respond(device, SSDP::NONE);
                device->_pending = false;
            }
        }

        timerPeriod(0);        
    }
    else
        yield();
}

/*
** Send Notification Broadcast
*/
void IOTSSDP::iotNotify(UPNPDevice *device, ssdp_method_t method)
{
    if (_state == IOT_RUNNING)
    {
        if (method == SSDP::NONE)
            method = SSDP::ALIVE;
        _respond(device, method);
    }
}

/*
** Add device to manage
*/
void IOTSSDP::addDevice(UPNPDevice *device)
{
    if (!device)
        return;

    if (!_lastDevice)
    {
        _firstDevice = device;
        _lastDevice = device;
    }
    else
    {
        _lastDevice->nextDevice(device);
        _lastDevice = device;
    }

    if (_iotMaster != nullptr)
        _iotMaster->addFunction(device);
    device->_sdpServer = this;
}

void IOTSSDP::_respond(UPNPDevice *device, ssdp_method_t method)
{
    char buffer[1460];
    char valueBuffer[strlen(_ssdp_notify_template) + 10];
    String surl = device->upnpSchemaURL();    
    String st_nt = device->upnpDeviceType();
    const char *nts = (method == SSDP::BYEBYE ? "byebye" : method == SSDP::UPDATE ? "update" : "alive");

    const char *schema = surl.c_str();    
    if (*schema == '/')
        schema++;
        
    if (method == SSDP::NONE && device->upnpSearchType() != "")
        st_nt = device->upnpSearchType();

    sprintf(valueBuffer, (method == SSDP::NONE) ? _ssdp_response_template : _ssdp_notify_template, nts);

    int len = snprintf(buffer, sizeof(buffer),
                       _ssdp_packet_template,
                       valueBuffer,
                       device->upnpInterval(),
                       IOT_VERSION,
                       device->upnpModelName().c_str(), device->upnpModelNumber().c_str(),
                       device->upnpUUID().c_str(), device->upnpDeviceType().c_str(),
                       (method == SSDP::NONE) ? "ST" : "NT", st_nt.c_str(),
                       WiFi.localIP().toString().c_str(),
                       device->upnpPort(),
                       schema,
                       (device->_ssdpHeader != nullptr) ? device->_ssdpHeader : strNull);

    if (method == SSDP::NONE)
    {
        if (!_udpServer.beginPacket(_pendingAddr, _pendingPort))
            return;
        device->_pending = false;           
        ESP_LOGD(_tag, "Response: %s:%d - [%s] %s",
                 _udpServer.remoteIP().toString().c_str(), _udpServer.remotePort(), 
                 _st.c_str(),
                 device->upnpDeviceType().c_str());
    }
    else if (!_udpServer.beginMulticastPacket())
    {
        return;
    }
    else
        ESP_LOGD(_tag, "NOTIFY (%s): %s:%d - %s", nts,
                 _udpServer.remoteIP().toString().c_str(), _udpServer.remotePort(), device->upnpDeviceType().c_str());

    ESP_LOGV(_tag, "\n%s", buffer);

    if (!_udpServer.write((const uint8_t *)&buffer[0], len) == len)
        ESP_LOGE(_tag, "Packet not Sent");
    _udpServer.endPacket();
}

bool IOTSSDP::_parsePacket(void)
{
    typedef enum { START,
                   MAN,
                   ST,
                   MX,
                   UNKNOWN } headers;

    headers header = START;

    String token;
    _st = "";

    // get message type
    int res = _parseToken(&token, true, false);
    if ((res <= 0) || token != "M-SEARCH")
    {
        ESP_LOGV(_tag, "REJECT METHOD: %s", token.c_str());
        _bailRead();
        return false;
    }

    // Get URI
    res = _parseToken(&token, true, false);
    if ((res <= 0) || token != "*")
    {
        ESP_LOGV(_tag, "REJECT URI: %s", token.c_str());
        _bailRead();
        return false;
    }

    // Eat protocol (HTTP/1.1)
    res = _parseToken(NULL, false, false);
    if (res <= 0)
    {
        _bailRead();
        return false;
    }

    while (_udpServer.available() > 0)
    {
        res = _parseToken(&token, header == START, header == START);

        if (res < 0 && header == START)
        {
            break;
        }

        switch (header)
        {
        case START:
            if (token.equalsIgnoreCase("MAN"))
                header = MAN;
            else if (token.equalsIgnoreCase("ST"))
                header = ST;
            else if (token.equalsIgnoreCase("MX"))
                header = MX;
            else
            {
                header = UNKNOWN;
                //ESP_LOGV(_tag, "UNKNOWN HEADER: %s", token.c_str());
            }
            break;

        case MAN:
            if (token != "\"ssdp:discover\"")
            {
                ESP_LOGV(_tag, "REJECT MAN: %s", token.c_str());
                _bailRead();
                return false;
            }
            header = START;
            break;

        case ST:
            _st = token;
            header = START;
            break;

        case MX:
            _mx = atoi(token.c_str());
            header = START;
            break;

        case UNKNOWN:
            header = START;
            break;
        }
    }

    // something broke during parsing of the message
    if (header != START)
        _bailRead();

    ESP_LOGV(_tag, "SSDP: %s (MX=%d)", _st.c_str(), _mx);
    return true;
}

/*
** writes the next token into token if token is not NULL
** returns -1 on message end, otherwise returns
*/
int IOTSSDP::_parseToken(String *token, bool break_on_space, bool break_on_colon)
{
    if (token)
        *token = "";
    bool token_found = false;
    int cr_found = 0;

    while (_udpServer.available() > 0)
    {
        char next = _udpServer.read();
        switch (next)
        {
        case '\r':
        case '\n':
            cr_found++;
            if (cr_found == 3)
            {
                // end of message reached
                return -1;
            }
            if (token_found)
            {
                // end of token reached
                return _udpServer.available();
            }
            continue;

        case ' ':
            // only treat spaces as part of text if they're not leading
            if (!token_found)
            {
                cr_found = 0;
                continue;
            }
            if (!break_on_space)
            {
                break;
            }
            cr_found = 0;

            // end of token reached
            return _udpServer.available();

        case ':':
            // only treat colons as part of text if they're not leading
            if (!token_found)
            {
                cr_found = 0;
                continue;
            }
            if (!break_on_colon)
            {
                break;
            }
            cr_found = 0;

            // end of token reached
            return _udpServer.available();

        default:
            cr_found = 0;
            token_found = true;
            break;
        }

        if (token)
        {
            (*token) += next;
        }
    }

    return 0;
}

void IOTSSDP::_bailRead()
{
    while (_parseToken(NULL, true, true) > 0)
        ;
    _st = "";
}

/******************************************************************************/