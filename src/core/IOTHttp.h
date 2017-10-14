/*
** EasyIOT - (HTTP) Really Simple Web Server Class
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
#ifndef _IOT_HTTP_H
#define _IOT_HTTP_H

#include <Arduino.h>
#include <esp_log.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESPmDNS.h>

/*
** Equates and Defintions
*/
#define HTTP_DOWNLOAD_UNIT_SIZE 1460

#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 2048
#endif

#define HTTP_MAX_DATA_WAIT 1000  //ms to wait for the client to send the request
#define HTTP_MAX_POST_WAIT 1000  //ms to wait for POST data to arrive
#define HTTP_MAX_SEND_WAIT 5000  //ms to wait for data chunk to be ACKed
#define HTTP_MAX_CLOSE_WAIT 2000 //ms to wait for the client to close the connection

#define CONTENT_LENGTH_UNKNOWN ((size_t)-1)
#define CONTENT_LENGTH_NOT_SET ((size_t)-2)

// State Values
//
#define IOT_HTTP_STOPPED 0
#define IOT_HTTP_RUNNING 1
#define IOT_HTTP_ERROR 2

enum HTTPMethod
{
    HTTP_ANY,
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_PATCH,
    HTTP_DELETE,
    HTTP_OPTIONS
};

enum HTTPUploadStatus
{
    UPLOAD_FILE_START,
    UPLOAD_FILE_WRITE,
    UPLOAD_FILE_END,
    UPLOAD_FILE_ABORTED
};

enum HTTPClientStatus
{
    HC_NONE,
    HC_WAIT_READ,
    HC_WAIT_CLOSE
};

/*
** Forward References
*/
class IOTHTTP;
class IOTFunction;
#include <FS.h>

/*
** Upload Information
*/
typedef struct
{
    HTTPUploadStatus status;
    String filename;
    String name;
    String type;
    size_t totalSize;   // file size
    size_t currentSize; // size of data currently in buf
    uint8_t buf[HTTP_UPLOAD_BUFLEN];
} HTTPUpload;

#include "http/HTTPHandler.h"

/*
** Simple Web Server Class
*/
class IOTHTTP : protected WiFiServer
{
  public:
    typedef std::function<void(IOTHTTP &)> http_callback_t;

    friend http_callback_t;
    friend class HTTPHandler;
    friend class IOTFunction;
    
    IOTHTTP(const char *tag, uint16_t port, bool mdns = true);
    ~IOTHTTP();

    const char * iotTag(void) { return _tag; }
    inline void iotDebug(esp_log_level_t level) { esp_log_level_set(_tag, level); }

    void webStartup(void);
    void webShutdown(void);
    void webService(void);
    uint16_t webPort(void) { return _port; }   
    void webHandler(HTTPHandler *handler);
    void webAuthenticate(void);
    bool webCredentials(const char *username, const char *password);
    
    void on(const String &uri, http_callback_t fn);
    void on(const String &uri, HTTPMethod method, http_callback_t fn);
    void on(const String &uri, HTTPMethod method, http_callback_t fn, http_callback_t ufn);
    void onFile(const char* uri, FS& fs, const char* path, const char* cache_header);
    void on404(http_callback_t fn) { _404Handler = fn; }
    void onUpload(http_callback_t fn) { _uploadHandler = fn; }

    String uri(void) { return _currentUri; }
    HTTPMethod method(void) { return _currentMethod; }
    WiFiClient client(void) { return _currentClient; }
    HTTPUpload &upload(void) { return _currentUpload; }

    int args(void) { return _currentArgCount; }
    bool hasArg(String name);
    String arg(String name);
    String arg(int i);
    String argName(int i);

    int headers(void) { return _headerKeysCount; }
    bool hasHeader(String name);
    void collectHeaders(const char *headerKeys[], const size_t headerKeysCount);
    String hostHeader(void) { return _hostHeader; }   
    String header(String name);
    String header(int i);
    String headerName(int i);

    void sendHeader(const String &name, const String &value, bool first = false);
    void sendContent(const String &content);

    bool send(int code, const char *content_type = NULL, const String &content = String(""));
    bool send(int code, char *content_type, const String &content);
    bool send(int code, const String &content_type, const String &content);

    void setContentLength(size_t contentLength) { _contentLength = contentLength; }    
    static String urlDecode(const String &text);

    template <typename T>
    size_t streamFile(T &file, const String &contentType)
    {
        setContentLength(file.size());

        if (String(file.name()).endsWith(".gz") &&
            contentType != MIME_TYPE_GZIP &&
            contentType != MIME_TYPE_DATA)
        {
            sendHeader("Content-Encoding", "gzip");
        }
        
        send(200, contentType, "");
        return _currentClient.write(file);
    }

private:
    struct RequestArgument
    {
        String key;
        String value;
    };

    void _addRequestHandler(HTTPHandler *handler);
    bool _collectHeader(const char *headerName, const char *headerValue);
    void _prepareHeader(String &response, int code, const char *content_type, size_t contentLength);
    void _handleRequest(void); 
       
    String _methodToString(int method);    
    String _responseCodeToString(int code);

    bool _parseRequest(WiFiClient &client);
    void _parseArguments(String data);
    bool _parseForm(WiFiClient &client, String boundary, uint32_t len);
    bool _parseFormUploadAborted();
    void _uploadWriteByte(uint8_t b);
    uint8_t _uploadReadByte(WiFiClient &client);

    HTTPHandler *_currentHandler;
    HTTPHandler *_firstHandler;
    HTTPHandler *_lastHandler;
    http_callback_t _404Handler;
    http_callback_t _uploadHandler;

    WiFiClient _currentClient;
    HTTPMethod _currentMethod;
    String _currentUri;
    uint8_t _currentVersion;
    HTTPClientStatus _currentStatus;
    unsigned long _statusChange;

    int _currentArgCount;
    RequestArgument *_currentArgs;
    HTTPUpload _currentUpload;

    int _headerKeysCount;
    RequestArgument *_currentHeaders;
    size_t _contentLength;
    String _responseHeaders;

    String _hostHeader;
    bool _chunked;

    const char *_tag;
    uint8_t _state;
    uint16_t _port;
    bool _bonjour;
};

#endif // _IOT_HTTP_H

/******************************************************************************/