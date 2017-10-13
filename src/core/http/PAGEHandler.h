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
#ifndef _IOT_PAGE_HANDLER_H
#define _IOT_PAGE_HANDLER_H
#include "HTTPHandler.h"

class PAGEHandler : public HTTPHandler
{
  public:
    PAGEHandler(IOTHTTP::http_callback_t fn, IOTHTTP::http_callback_t ufn, const String &uri, HTTPMethod method)
        : _fn(fn), _ufn(ufn), _uri(uri), _method(method)
    {
    }

    bool canHandle(HTTPMethod requestMethod, String requestUri) override
    {
        if (_method != HTTP_ANY && _method != requestMethod)
            return false;

        if (requestUri != _uri)
            return false;

        return true;
    }

    bool canUpload(String requestUri) override
    {
        if (!_ufn || !canHandle(HTTP_POST, requestUri))
            return false;

        return true;
    }

    bool handle(IOTHTTP &server, HTTPMethod requestMethod, String requestUri) override
    {
        (void)server;
        if (!canHandle(requestMethod, requestUri))
            return false;

        _fn(server);
        return true;
    }

    void upload(IOTHTTP &server, String requestUri, HTTPUpload &upload) override
    {
        (void)server;
        (void)upload;
        if (canUpload(requestUri))
            _ufn(server);
    }

  protected:
    IOTHTTP::http_callback_t _fn;
    IOTHTTP::http_callback_t _ufn;
    HTTPMethod _method;
    String _uri;
};

#endif // _IOT_PAGE_HANDLER_H

/******************************************************************************/