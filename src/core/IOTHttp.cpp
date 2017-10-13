/*
** EasyIOT - (HTTP) Simple Web Server Class
**
** This is based upon the ESP8266WebServer library, 
** Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
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
#include "IOTHttp.h"
#include "http/PAGEHandler.h"
#include "http/FILEHandler.h"

#include <libb64/cencode.h>
#include <FS.h>

const char *AUTHORIZATION_HEADER = "Authorization";

/*
** Class Construction
*/
IOTHTTP::IOTHTTP(const char *tag, uint16_t port, bool mdns)
    : WiFiServer(port),
      _tag(tag),
      _port(port),
      _bonjour(mdns),
      _state(IOT_HTTP_STOPPED),
      _currentMethod(HTTP_ANY),
      _currentVersion(0),
      _currentStatus(HC_NONE),
      _statusChange(0),
      _currentHandler(0),
      _firstHandler(0),
      _lastHandler(0),
      _currentArgCount(0),
      _currentArgs(0),
      _headerKeysCount(0),
      _currentHeaders(0),
      _contentLength(0),
      _chunked(false)
{
    ESP_LOGI(_tag, "Created HTTP (%d) Server", _port);    
}

/*
** Class Deconstruction
*/
IOTHTTP::~IOTHTTP()
{
    webShutdown();
    if (_currentHeaders)
        delete[] _currentHeaders;
    _headerKeysCount = 0;
    HTTPHandler *handler = _firstHandler;

    while (handler)
    {
        HTTPHandler *next = handler->nextHandler();
        delete handler;
        handler = next;
    }
    close();
}

/*
** Service Startup
*/
void IOTHTTP::webStartup(void)
{
    if (_state != IOT_HTTP_STOPPED)
        return;

    ESP_LOGI(_tag, "Starting HTTP Server: %d", _port);

    _currentStatus = HC_NONE;
    begin();
    if (!_headerKeysCount)
        collectHeaders(0, 0);

    if (_bonjour) {
        delay(500);
        MDNS.addService("_http", "_tcp", _port);
    }

    _state = IOT_HTTP_RUNNING;
}

/*
** Service Shutdown
*/
void IOTHTTP::webShutdown(void)
{
    if (_state == IOT_HTTP_STOPPED)
        return;

    ESP_LOGI(_tag, "Stopping HTTP Server");
    stop();

    _state = IOT_HTTP_STOPPED;
}

/*
** Service Update
*/
void IOTHTTP::webService(void)
{
    if (_state != IOT_HTTP_RUNNING)
        return;

    if (_currentStatus == HC_NONE)
    {
        WiFiClient client = available();
        if (!client)
        {
            return;
        }

        ESP_LOGV(_tag, "Client Connected: %s:%d", client.remoteIP().toString().c_str(), client.remotePort());

        _currentClient = client;
        _currentStatus = HC_WAIT_READ;
        _statusChange = millis();
    }

    if (!_currentClient.connected())
    {
        _currentClient = WiFiClient();
        _currentStatus = HC_NONE;
        return;
    }

    // Wait for data from client to become available
    if (_currentStatus == HC_WAIT_READ)
    {
        if (!_currentClient.available())
        {
            if (millis() - _statusChange > HTTP_MAX_DATA_WAIT)
            {
                _currentClient = WiFiClient();
                _currentStatus = HC_NONE;
            }
            yield();
            return;
        }

        ESP_LOGV(_tag, "Content Size: %d", _currentClient.available());
        
        if (!_parseRequest(_currentClient))
        {
            _currentClient = WiFiClient();
            _currentStatus = HC_NONE;
            ESP_LOGV(_tag, "Parsing Request Failed!");
            return;
        }

        _currentClient.setTimeout(HTTP_MAX_SEND_WAIT);
        _contentLength = CONTENT_LENGTH_NOT_SET;
        _handleRequest();

        if (!_currentClient.connected())
        {
            _currentClient = WiFiClient();
            _currentStatus = HC_NONE;
            return;
        }
        else
        {
            _currentStatus = HC_WAIT_CLOSE;
            _statusChange = millis();
            return;
        }
    }

    if (_currentStatus == HC_WAIT_CLOSE)
    {
        if (millis() - _statusChange > HTTP_MAX_CLOSE_WAIT)
        {
            _currentClient = WiFiClient();
            _currentStatus = HC_NONE;
        }
        else
        {
            yield();
            return;
        }
    }
}

/*
** HTTP Handlers
*/
void IOTHTTP::webHandler(HTTPHandler *handler)
{
    _addRequestHandler(handler);
}

void IOTHTTP::_addRequestHandler(HTTPHandler *handler)
{
    if (!_lastHandler)
    {
        _firstHandler = handler;
        _lastHandler = handler;
    }
    else
    {
        _lastHandler->nextHandler(handler);
        _lastHandler = handler;
    }
}

/*
** Basic HTTP Authentication
*/
void IOTHTTP::webAuthenticate()
{
    sendHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
    send(401);
}

bool IOTHTTP::webCredentials(const char *username, const char *password)
{
    if (hasHeader(AUTHORIZATION_HEADER))
    {
        String authReq = header(AUTHORIZATION_HEADER);

        if (authReq.startsWith("Basic"))
        {
            authReq = authReq.substring(6);
            authReq.trim();
            char toencodeLen = strlen(username) + strlen(password) + 1;
            char *toencode = new char[toencodeLen + 1];

            if (toencode == NULL)
            {
                authReq = String();
                return false;
            }

            char *encoded = new char[base64_encode_expected_len(toencodeLen) + 1];

            if (encoded == NULL)
            {
                authReq = String();
                delete[] toencode;
                return false;
            }

            sprintf(toencode, "%s:%s", username, password);

            if (base64_encode_chars(toencode, toencodeLen, encoded) > 0 && authReq.equals(encoded))
            {
                authReq = String();
                delete[] toencode;
                delete[] encoded;
                return true;
            }
            delete[] toencode;
            delete[] encoded;
        }
        authReq = String();
    }
    return false;
}

/*
** On Handlers
*/
void IOTHTTP::on(const String &uri, http_callback_t fn)
{
    on(uri, HTTP_ANY, fn);
}

void IOTHTTP::on(const String &uri, HTTPMethod method, http_callback_t fn)
{
    on(uri, method, fn, _uploadHandler);
}

void IOTHTTP::on(const String &uri, HTTPMethod method, http_callback_t fn, http_callback_t ufn)
{
    ESP_LOGD(_tag, "On Handler: %s", uri.c_str());    
    _addRequestHandler(new PAGEHandler(fn, ufn, uri, method));
}

void IOTHTTP::onFile(const char *uri, FS &fs, const char *path, const char *cache_header)
{
    _addRequestHandler(new FILEHandler(fs, path, uri, cache_header));
}

/*
** Arguments
*/
bool IOTHTTP::hasArg(String name)
{
    for (int i = 0; i < _currentArgCount; ++i)
    {
        if (_currentArgs[i].key == name)
            return true;
    }
    return false;
}

String IOTHTTP::arg(String name)
{
    for (int i = 0; i < _currentArgCount; ++i)
    {
        if (_currentArgs[i].key == name)
            return _currentArgs[i].value;
    }
    return String();
}

String IOTHTTP::arg(int i)
{
    if (i < _currentArgCount)
        return _currentArgs[i].value;
    return String();
}

String IOTHTTP::argName(int i)
{
    if (i < _currentArgCount)
        return _currentArgs[i].key;
    return String();
}

/*
** Headers
*/
bool IOTHTTP::hasHeader(String name)
{
    for (int i = 0; i < _headerKeysCount; ++i)
    {
        if ((_currentHeaders[i].key.equalsIgnoreCase(name)) && (_currentHeaders[i].value.length() > 0))
            return true;
    }
    return false;
}

void IOTHTTP::collectHeaders(const char *headerKeys[], const size_t headerKeysCount)
{
    _headerKeysCount = headerKeysCount + 1;
    if (_currentHeaders)
        delete[] _currentHeaders;
    _currentHeaders = new RequestArgument[_headerKeysCount];
    _currentHeaders[0].key = AUTHORIZATION_HEADER;
    for (int i = 1; i < _headerKeysCount; i++)
    {
        _currentHeaders[i].key = headerKeys[i - 1];
    }
}

String IOTHTTP::header(String name)
{
    for (int i = 0; i < _headerKeysCount; ++i)
    {
        if (_currentHeaders[i].key.equalsIgnoreCase(name))
            return _currentHeaders[i].value;
    }
    return String();
}

String IOTHTTP::header(int i)
{
    if (i < _headerKeysCount)
        return _currentHeaders[i].value;
    return String();
}

String IOTHTTP::headerName(int i)
{
    if (i < _headerKeysCount)
        return _currentHeaders[i].key;
    return String();
}

void IOTHTTP::_prepareHeader(String &response, int code, const char *content_type, size_t contentLength)
{
    response = "HTTP/1." + String(_currentVersion) + " ";
    response += String(code);
    response += " ";
    response += _responseCodeToString(code);
    response += "\r\n";

    if (!content_type)
        content_type = MIME_TYPE_HTML;

    sendHeader("Content-Type", content_type, true);
    if (_contentLength == CONTENT_LENGTH_NOT_SET)
    {
        sendHeader("Content-Length", String(contentLength));
    }
    else if (_contentLength != CONTENT_LENGTH_UNKNOWN)
    {
        sendHeader("Content-Length", String(_contentLength));
    }
    else if (_contentLength == CONTENT_LENGTH_UNKNOWN && _currentVersion)
    { //HTTP/1.1 or above client
        //let's do chunked
        _chunked = true;
        sendHeader("Accept-Ranges", "none");
        sendHeader("Transfer-Encoding", "chunked");
    }
    sendHeader("Connection", "close");

    response += _responseHeaders;
    response += "\r\n";
    _responseHeaders = String();
}

/*
** Senders
*/
void IOTHTTP::sendHeader(const String &name, const String &value, bool first)
{
    String headerLine = name;
    headerLine += ": ";
    headerLine += value;
    headerLine += "\r\n";

    if (first)
    {
        _responseHeaders = headerLine + _responseHeaders;
    }
    else
    {
        _responseHeaders += headerLine;
    }
}

void IOTHTTP::sendContent(const String &content)
{
    const char *footer = "\r\n";
    size_t len = content.length();

    if (_chunked)
    {
        char *chunkSize = (char *)malloc(11);
        if (chunkSize)
        {
            sprintf(chunkSize, "%x%s", len, footer);
            _currentClient.write(chunkSize, strlen(chunkSize));
            free(chunkSize);
        }
    }

    _currentClient.write(content.c_str(), len);

    if (_chunked)
    {
        _currentClient.write(footer, 2);
    }
}

void IOTHTTP::send(int code, char *content_type, const String &content)
{
    send(code, (const char *)content_type, content);
}

void IOTHTTP::send(int code, const String &content_type, const String &content)
{
    send(code, (const char *)content_type.c_str(), content);
}

void IOTHTTP::send(int code, const char *content_type, const String &content)
{
    String header;
    
    // Can we asume the following?
    //if(code == 200 && content.length() == 0 && _contentLength == CONTENT_LENGTH_NOT_SET)
    //  _contentLength = CONTENT_LENGTH_UNKNOWN;
    _prepareHeader(header, code, content_type, content.length());
    _currentClient.write(header.c_str(), header.length());

    if (content.length())
        sendContent(content);
    
    ESP_LOGD(_tag, "Served (%d %d %s): %s:%d - %s", _currentMethod, code, _responseCodeToString(code).c_str(),
        _currentClient.remoteIP().toString().c_str(), _currentClient.remotePort(), _currentUri.c_str());
}

/*
** Internal Handlers
*/
void IOTHTTP::_handleRequest()
{
    bool handled = false;
    
    if (!_currentHandler)
    {
        //ESP_LOGV(_tag, "Request handler not found: %s", _currentUri.c_str());
    }
    else
    {
        handled = _currentHandler->handle(*this, _currentMethod, _currentUri);

        if (!handled)
        {
            ESP_LOGV(_tag, "Request handler failed to handle request");
        }
    }

    if (!handled)
    {
        if (_404Handler)
        {
            _404Handler(*this);
        }
        else
        {
            send(404, MIME_TYPE_TEXT, String("Not found: ") + _currentUri);
        }
    }

    _currentUri = String();
}

String IOTHTTP::_responseCodeToString(int code)
{
    switch (code)
    {
    case 100:
        return F("Continue");
    case 101:
        return F("Switching Protocols");
    case 200:
        return F("OK");
    case 201:
        return F("Created");
    case 202:
        return F("Accepted");
    case 203:
        return F("Non-Authoritative Information");
    case 204:
        return F("No Content");
    case 205:
        return F("Reset Content");
    case 206:
        return F("Partial Content");
    case 300:
        return F("Multiple Choices");
    case 301:
        return F("Moved Permanently");
    case 302:
        return F("Found");
    case 303:
        return F("See Other");
    case 304:
        return F("Not Modified");
    case 305:
        return F("Use Proxy");
    case 307:
        return F("Temporary Redirect");
    case 400:
        return F("Bad Request");
    case 401:
        return F("Unauthorized");
    case 402:
        return F("Payment Required");
    case 403:
        return F("Forbidden");
    case 404:
        return F("Not Found");
    case 405:
        return F("Method Not Allowed");
    case 406:
        return F("Not Acceptable");
    case 407:
        return F("Proxy Authentication Required");
    case 408:
        return F("Request Time-out");
    case 409:
        return F("Conflict");
    case 410:
        return F("Gone");
    case 411:
        return F("Length Required");
    case 412:
        return F("Precondition Failed");
    case 413:
        return F("Request Entity Too Large");
    case 414:
        return F("Request-URI Too Large");
    case 415:
        return F("Unsupported Media Type");
    case 416:
        return F("Requested range not satisfiable");
    case 417:
        return F("Expectation Failed");
    case 500:
        return F("Internal Server Error");
    case 501:
        return F("Not Implemented");
    case 502:
        return F("Bad Gateway");
    case 503:
        return F("Service Unavailable");
    case 504:
        return F("Gateway Time-out");
    case 505:
        return F("HTTP Version not supported");
    default:
        return "";
    }
}

/*
** Parsers
*/
static char *readBytesWithTimeout(WiFiClient &client, size_t maxLength, size_t &dataLength, int timeout_ms)
{
    char *buf = nullptr;
    dataLength = 0;
    while (dataLength < maxLength)
    {
        int tries = timeout_ms;
        size_t newLength;
        while (!(newLength = client.available()) && tries--)
            delay(1);

        if (!newLength)
        {
            break;
        }
        if (!buf)
        {
            buf = (char *)malloc(newLength + 1);
            if (!buf)
            {
                return nullptr;
            }
        }
        else
        {
            char *newBuf = (char *)realloc(buf, dataLength + newLength + 1);
            if (!newBuf)
            {
                free(buf);
                return nullptr;
            }
            buf = newBuf;
        }
        client.readBytes(buf + dataLength, newLength);
        dataLength += newLength;
        buf[dataLength] = '\0';
    }
    return buf;
}

bool IOTHTTP::_parseRequest(WiFiClient &client)
{
    // Read the first line of HTTP request
    // Handle bad apps who only send LF!
    String req = client.readStringUntil('\n');
    if (req[req.length() - 1] == '\r') {
        req.trim();
    }
    
    // reset header value
    for (int i = 0; i < _headerKeysCount; ++i)
    {
        _currentHeaders[i].value = String();
    }

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);

    if (addr_start == -1 || addr_end == -1)
    {
        ESP_LOGD(_tag, "Invalid Request: %s", req.c_str());
        return false;
    }

    String methodStr = req.substring(0, addr_start);
    String url = req.substring(addr_start + 1, addr_end);
    String versionEnd = req.substring(addr_end + 8);
    _currentVersion = atoi(versionEnd.c_str());
    String searchStr = "";
    int hasSearch = url.indexOf('?');

    if (hasSearch != -1)
    {
        searchStr = urlDecode(url.substring(hasSearch + 1));
        url = url.substring(0, hasSearch);
    }

    _currentUri = url;
    _chunked = false;

    HTTPMethod method = HTTP_GET;
    if (methodStr == "POST")
    {
        method = HTTP_POST;
    }
    else if (methodStr == "DELETE")
    {
        method = HTTP_DELETE;
    }
    else if (methodStr == "OPTIONS")
    {
        method = HTTP_OPTIONS;
    }
    else if (methodStr == "PUT")
    {
        method = HTTP_PUT;
    }
    else if (methodStr == "PATCH")
    {
        method = HTTP_PATCH;
    }
    _currentMethod = method;

    ESP_LOGD(_tag, "Method: %s URL: %s Search: %s", methodStr.c_str(), url.c_str(), searchStr.c_str());
    
    // Attach handler
    HTTPHandler *handler;

    for (handler = _firstHandler; handler; handler = handler->nextHandler())
    {
        if (handler->canHandle(_currentMethod, _currentUri))
            break;
    }
    _currentHandler = handler;

    String formData;

    // Below is needed only when POST type request
    if (method == HTTP_POST || method == HTTP_PUT || method == HTTP_PATCH || method == HTTP_DELETE)
    {
        String boundaryStr;
        String headerName;
        String headerValue;
        bool isForm = false;
        bool isEncoded = false;
        uint32_t contentLength = 0;

        
        // Parse headers
        while (1)
        {
            // Handle bad apps who only send LF!
            String req = client.readStringUntil('\n');
            if (req[req.length() - 1] == '\r') {
                req.trim();
            }
            
            if (req == "")
                break; //no more headers

            int headerDiv = req.indexOf(':');

            if (headerDiv == -1)
            {
                break;
            }

            headerName = req.substring(0, headerDiv);
            headerValue = req.substring(headerDiv + 1);
            headerValue.trim();
            _collectHeader(headerName.c_str(), headerValue.c_str());

            ESP_LOGV(_tag, "Header: %s = %s", headerName.c_str(), headerValue.c_str());

            if (headerName.equalsIgnoreCase("Content-Type"))
            {
                if (headerValue.startsWith("text/plain"))
                {
                    isForm = false;
                }
                else if (headerValue.startsWith("application/x-www-form-urlencoded"))
                {
                    isForm = false;
                    isEncoded = true;
                }
                else if (headerValue.startsWith("multipart/"))
                {
                    boundaryStr = headerValue.substring(headerValue.indexOf('=') + 1);
                    isForm = true;
                }
            }
            else if (headerName.equalsIgnoreCase("Content-Length"))
            {
                contentLength = headerValue.toInt();
            }
            else if (headerName.equalsIgnoreCase("Host"))
            {
                _hostHeader = headerValue;
            }
        }

        if (!isForm)
        {           
            size_t plainLength;
            char *plainBuf = readBytesWithTimeout(client, contentLength, plainLength, HTTP_MAX_POST_WAIT);
            
            if (plainLength < contentLength)
            {
                free(plainBuf);
                return false;
            }

            if (contentLength > 0)
            {
                if (searchStr != "")
                    searchStr += '&';
                if (isEncoded)
                {
                    //url encoded form
                    String decoded = urlDecode(plainBuf);
                    size_t decodedLen = decoded.length();
                    memcpy(plainBuf, decoded.c_str(), decodedLen);
                    plainBuf[decodedLen] = 0;
                    searchStr += plainBuf;
                }

                _parseArguments(searchStr);

                if (!isEncoded)
                {
                    //plain post json or other data
                    RequestArgument &arg = _currentArgs[_currentArgCount++];
                    arg.key = "plain";
                    arg.value = String(plainBuf);
                }

                ESP_LOGV(_tag, "Plain: %s", plainBuf);

                free(plainBuf);
            }
        }

        if (isForm)
        {
            _parseArguments(searchStr);
            if (!_parseForm(client, boundaryStr, contentLength))
            {
                return false;
            }
        }
    }
    else
    {
        String headerName;
        String headerValue;

        // Parse headers
        while (1)
        {
            // Handle bad apps who only send LF!
            String req = client.readStringUntil('\n');
            if (req[req.length() - 1] == '\r') {
                req.trim();
            }
            
            if (req == "")
                break; //no moar headers

            int headerDiv = req.indexOf(':');
            if (headerDiv == -1)
            {
                break;
            }

            headerName = req.substring(0, headerDiv);
            headerValue = req.substring(headerDiv + 2);
            _collectHeader(headerName.c_str(), headerValue.c_str());

            ESP_LOGV(_tag, "Header: %s = %s", headerName.c_str(), headerValue.c_str());

            if (headerName.equalsIgnoreCase("Host"))
            {
                _hostHeader = headerValue;
            }
        }
        _parseArguments(searchStr);
    }
    
    client.flush();

    //ESP_LOGV(_tag, "Request: %s Args: %s", url.c_str(), searchStr.c_str());

    return true;
}

bool IOTHTTP::_collectHeader(const char *headerName, const char *headerValue)
{
    for (int i = 0; i < _headerKeysCount; i++)
    {
        if (_currentHeaders[i].key.equalsIgnoreCase(headerName))
        {
            _currentHeaders[i].value = headerValue;
            return true;
        }
    }
    return false;
}

void IOTHTTP::_parseArguments(String data)
{
    ESP_LOGV(_tag, "Args: %s", data.c_str());

    if (_currentArgs)
        delete[] _currentArgs;
    _currentArgs = 0;

    if (data.length() == 0)
    {
        _currentArgCount = 0;
        _currentArgs = new RequestArgument[1];
        return;
    }
    _currentArgCount = 1;

    for (int i = 0; i < (int)data.length();)
    {
        i = data.indexOf('&', i);
        if (i == -1)
            break;
        ++i;
        ++_currentArgCount;
    }

    ESP_LOGV(_tag, "Args Count: %d", _currentArgCount);

    _currentArgs = new RequestArgument[_currentArgCount + 1];

    int pos = 0;
    int iarg;

    for (iarg = 0; iarg < _currentArgCount;)
    {
        int equal_sign_index = data.indexOf('=', pos);
        int next_arg_index = data.indexOf('&', pos);

        ESP_LOGV(_tag, "Pos: %d =@: %d &@: %d", pos, equal_sign_index, next_arg_index);

        if ((equal_sign_index == -1) || ((equal_sign_index > next_arg_index) && (next_arg_index != -1)))
        {
            ESP_LOGV(_tag, "Arg: %d Missing Value", iarg);

            if (next_arg_index == -1)
                break;
            pos = next_arg_index + 1;
            continue;
        }

        RequestArgument &arg = _currentArgs[iarg];
        arg.key = data.substring(pos, equal_sign_index);
        arg.value = data.substring(equal_sign_index + 1, next_arg_index);

        ESP_LOGV(_tag, "Arg: %d Key: %s = %s", iarg, arg.key.c_str(), arg.value.c_str());

        ++iarg;
        if (next_arg_index == -1)
            break;
        pos = next_arg_index + 1;
    }

    _currentArgCount = iarg;
    ESP_LOGV(_tag, "Args Count: %d", _currentArgCount);
}

void IOTHTTP::_uploadWriteByte(uint8_t b)
{
    if (_currentUpload.currentSize == HTTP_UPLOAD_BUFLEN)
    {
        if (_currentHandler && _currentHandler->canUpload(_currentUri))
            _currentHandler->upload(*this, _currentUri, _currentUpload);
        _currentUpload.totalSize += _currentUpload.currentSize;
        _currentUpload.currentSize = 0;
    }
    _currentUpload.buf[_currentUpload.currentSize++] = b;
}

uint8_t IOTHTTP::_uploadReadByte(WiFiClient &client)
{
    int res = client.read();
    if (res == -1)
    {
        while (!client.available() && client.connected())
            yield();
        res = client.read();
    }
    return (uint8_t)res;
}

bool IOTHTTP::_parseForm(WiFiClient &client, String boundary, uint32_t len)
{
    (void)len;

    ESP_LOGV(_tag, "Parse Form, Boundary: %s Len: %s", boundary.c_str(), len);

    String line;
    int retry = 0;
    do
    {
        line = client.readStringUntil('\r');
        ++retry;
    } while (line.length() == 0 && retry < 3);

    client.readStringUntil('\n');

    // Start reading the form
    if (line == ("--" + boundary))
    {
        RequestArgument *postArgs = new RequestArgument[32];
        int postArgsLen = 0;
        while (1)
        {
            String argName;
            String argValue;
            String argType;
            String argFilename;
            bool argIsFile = false;

            line = client.readStringUntil('\r');
            client.readStringUntil('\n');
            if (line.length() > 19 && line.substring(0, 19).equalsIgnoreCase("Content-Disposition"))
            {
                int nameStart = line.indexOf('=');
                if (nameStart != -1)
                {
                    argName = line.substring(nameStart + 2);
                    nameStart = argName.indexOf('=');
                    if (nameStart == -1)
                    {
                        argName = argName.substring(0, argName.length() - 1);
                    }
                    else
                    {
                        argFilename = argName.substring(nameStart + 2, argName.length() - 1);
                        argName = argName.substring(0, argName.indexOf('"'));
                        argIsFile = true;

                        ESP_LOGV(_tag, "POST Arg, File: %s", argFilename.c_str());

                        // Use GET to set the filename if uploading using blob
                        if (argFilename == "blob" && hasArg("filename"))
                            argFilename = arg("filename");
                    }

                    ESP_LOGV(_tag, "POST Arg, Name: %s", argName.c_str());

                    argType = MIME_TYPE_TEXT;
                    line = client.readStringUntil('\r');
                    client.readStringUntil('\n');
                    if (line.length() > 12 && line.substring(0, 12).equalsIgnoreCase("Content-Type"))
                    {
                        argType = line.substring(line.indexOf(':') + 2);

                        // Skip next line
                        client.readStringUntil('\r');
                        client.readStringUntil('\n');
                    }

                    ESP_LOGV(_tag, "POST Arg, Type: %s", argType.c_str());

                    if (!argIsFile)
                    {
                        while (1)
                        {
                            line = client.readStringUntil('\r');
                            client.readStringUntil('\n');
                            if (line.startsWith("--" + boundary))
                                break;
                            if (argValue.length() > 0)
                                argValue += "\n";
                            argValue += line;
                        }

                        ESP_LOGV(_tag, "POST Arg, Value: %s", argValue.c_str());

                        RequestArgument &arg = postArgs[postArgsLen++];
                        arg.key = argName;
                        arg.value = argValue;

                        if (line == ("--" + boundary + "--"))
                        {
                            ESP_LOGV(_tag, "POST: Parsing Done");
                            break;
                        }
                    }
                    else
                    {
                        _currentUpload.status = UPLOAD_FILE_START;
                        _currentUpload.name = argName;
                        _currentUpload.filename = argFilename;
                        _currentUpload.type = argType;
                        _currentUpload.totalSize = 0;
                        _currentUpload.currentSize = 0;

                        ESP_LOGV(_tag, "Start File: %s Type: %s", _currentUpload.filename.c_str(), _currentUpload.type.c_str());

                        if (_currentHandler && _currentHandler->canUpload(_currentUri))
                            _currentHandler->upload(*this, _currentUri, _currentUpload);

                        _currentUpload.status = UPLOAD_FILE_WRITE;
                        uint8_t argByte = _uploadReadByte(client);

                    readfile:
                        while (argByte != 0x0D)
                        {
                            if (!client.connected())
                                return _parseFormUploadAborted();
                            _uploadWriteByte(argByte);
                            argByte = _uploadReadByte(client);
                        }

                        argByte = _uploadReadByte(client);

                        if (!client.connected())
                            return _parseFormUploadAborted();

                        if (argByte == 0x0A)
                        {
                            argByte = _uploadReadByte(client);

                            if (!client.connected())
                                return _parseFormUploadAborted();

                            if ((char)argByte != '-')
                            {
                                //continue reading the file
                                _uploadWriteByte(0x0D);
                                _uploadWriteByte(0x0A);
                                goto readfile;
                            }
                            else
                            {
                                argByte = _uploadReadByte(client);
                                if (!client.connected())
                                    return _parseFormUploadAborted();
                                if ((char)argByte != '-')
                                {
                                    //continue reading the file
                                    _uploadWriteByte(0x0D);
                                    _uploadWriteByte(0x0A);
                                    _uploadWriteByte((uint8_t)('-'));
                                    goto readfile;
                                }
                            }

                            uint8_t endBuf[boundary.length()];
                            client.readBytes(endBuf, boundary.length());

                            if (strstr((const char *)endBuf, boundary.c_str()) != NULL)
                            {
                                if (_currentHandler && _currentHandler->canUpload(_currentUri))
                                    _currentHandler->upload(*this, _currentUri, _currentUpload);

                                _currentUpload.totalSize += _currentUpload.currentSize;
                                _currentUpload.status = UPLOAD_FILE_END;

                                if (_currentHandler && _currentHandler->canUpload(_currentUri))
                                    _currentHandler->upload(*this, _currentUri, _currentUpload);
                                
                                ESP_LOGV(_tag, "End File: %s Type: %s Size: %d", 
                                    _currentUpload.filename.c_str(), _currentUpload.type.c_str(), _currentUpload.totalSize);
                                    
                                line = client.readStringUntil(0x0D);
                                client.readStringUntil(0x0A);
                                if (line == "--")
                                {
                                    ESP_LOGV(_tag, "POST: Parsing Done");
                                    break;
                                }
                                continue;
                            }
                            else
                            {
                                _uploadWriteByte(0x0D);
                                _uploadWriteByte(0x0A);
                                _uploadWriteByte((uint8_t)('-'));
                                _uploadWriteByte((uint8_t)('-'));
                                uint32_t i = 0;
                                while (i < boundary.length())
                                {
                                    _uploadWriteByte(endBuf[i++]);
                                }
                                argByte = _uploadReadByte(client);
                                goto readfile;
                            }
                        }
                        else
                        {
                            _uploadWriteByte(0x0D);
                            goto readfile;
                        }
                        break;
                    }
                }
            }
        }

        int iarg;
        int totalArgs = ((32 - postArgsLen) < _currentArgCount) ? (32 - postArgsLen) : _currentArgCount;
        
        for (iarg = 0; iarg < totalArgs; iarg++)
        {
            RequestArgument &arg = postArgs[postArgsLen++];
            arg.key = _currentArgs[iarg].key;
            arg.value = _currentArgs[iarg].value;
        }

        if (_currentArgs)
            delete[] _currentArgs;
        _currentArgs = new RequestArgument[postArgsLen];
        
        for (iarg = 0; iarg < postArgsLen; iarg++)
        {
            RequestArgument &arg = _currentArgs[iarg];
            arg.key = postArgs[iarg].key;
            arg.value = postArgs[iarg].value;
        }
        
        _currentArgCount = iarg;
        if (postArgs)
            delete[] postArgs;
        
        return true;
    }

    ESP_LOGV(_tag, "ERROR-Line: %s", line.c_str());
    
    return false;
}

String IOTHTTP::urlDecode(const String &text)
{
    String decoded = "";
    char temp[] = "0x00";
    unsigned int len = text.length();
    unsigned int i = 0;
    while (i < len)
    {
        char decodedChar;
        char encodedChar = text.charAt(i++);
        if ((encodedChar == '%') && (i + 1 < len))
        {
            temp[2] = text.charAt(i++);
            temp[3] = text.charAt(i++);

            decodedChar = strtol(temp, NULL, 16);
        }
        else
        {
            if (encodedChar == '+')
            {
                decodedChar = ' ';
            }
            else
            {
                decodedChar = encodedChar; // normal ascii char
            }
        }
        decoded += decodedChar;
    }
    return decoded;
}

bool IOTHTTP::_parseFormUploadAborted()
{
    _currentUpload.status = UPLOAD_FILE_ABORTED;
    if (_currentHandler && _currentHandler->canUpload(_currentUri))
        _currentHandler->upload(*this, _currentUri, _currentUpload);
    return false;
}
/******************************************************************************/
