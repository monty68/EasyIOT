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
#ifndef _IOT_HTTP_HANDLER_H
#define _IOT_HTTP_HANDLER_H

/*
** Common Mime Types
*/
#define MIME_TYPE_DATA "application/octet-stream"
#define MIME_TYPE_GZIP "application/x-gzip"
#define MIME_TYPE_TEXT "text/plain"
#define MIME_TYPE_HTML "text/html"
#define MIME_TYPE_JAVA "application/javascript"
#define MIME_TYPE_JSON "application/json"
#define MIME_TYPE_XML "text/xml"
#define MIME_TYPE_CSS "text/css"
#define MIME_TYPE_GIF "image/gif"
#define MIME_TYPE_ICO "image/x-icon"
#define MIME_TYPE_JPG "image/jpeg"
#define MIME_TYPE_SVG "image/svg+xml"
#define MIME_TYPE_PNG "image/png"

#define MIME_TYPE_JS MIME_TYPE_JAVA
#define MIME_TYPE_JPEG MIME_TYPE_JPG

/*
** HTTP Handler (Abstract) Class
*/
class HTTPHandler
{
  public:
    virtual ~HTTPHandler() {}
    
    virtual bool canHandle(HTTPMethod method, String uri)
    {
        (void)method;
        (void)uri;
        return false;
    }

    virtual bool canUpload(String uri)
    {
        (void)uri;
        return false;
    }

    virtual bool handle(IOTHTTP &server, HTTPMethod requestMethod, String requestUri)
    {
        (void)server;
        (void)requestMethod;
        (void)requestUri;
        return false;
    }

    virtual void upload(IOTHTTP &server, String requestUri, HTTPUpload &upload)
    {
        (void)server;
        (void)requestUri;
        (void)upload;
    }

    HTTPHandler *nextHandler() { return _httpNext; }
    void nextHandler(HTTPHandler *r) { _httpNext = r; }

  private:
    HTTPHandler *_httpNext = nullptr;
};

#endif // _IOT_HTTP_HANDLER_H

/******************************************************************************/