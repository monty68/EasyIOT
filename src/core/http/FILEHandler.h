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
#ifndef _IOT_FILE_HANDLER_H
#define _IOT_FILE_HANDLER_H

class FILEHandler : public HTTPHandler
{
  public:
    FILEHandler(FS &fs, const char *path, const char *uri, const char *cache_header)
        : _fs(fs), _uri(uri), _path(path), _cache_header(cache_header)
    {
        _isFile = fs.exists(path);
        
        ESP_LOGV("HTTP", "FILEHandler: path=%s uri=%s isFile=%d, cache_header=%s\r\n", path, uri, _isFile, cache_header);
        _baseUriLength = _uri.length();
    }

    bool httpCanHandle(HTTPMethod requestMethod, String requestUri) override
    {
        if (requestMethod != HTTP_GET)
            return false;

        if ((_isFile && requestUri != _uri) || !requestUri.startsWith(_uri))
            return false;

        return true;
    }

    bool httpHandle(IOTHTTP &server, HTTPMethod requestMethod, String requestUri) override
    {
        if (!httpCanHandle(requestMethod, requestUri))
            return false;

        ESP_LOGD(server.iotTag(), "FILEHandler: request=%s _uri=%s\r\n", requestUri.c_str(), _uri.c_str());

        String path(_path);

        if (!_isFile)
        {
            // Base URI doesn't point to a file.
            // If a directory is requested, look for index file.
            if (requestUri.endsWith("/"))
                requestUri += "index.htm";

            // Append whatever follows this URI in request to get the file path.
            path += requestUri.substring(_baseUriLength);
        }

        ESP_LOGD(server.iotTag(), "FILEHandler: path=%s, isFile=%d\r\n", path.c_str(), _isFile);

        String contentType = getContentType(path);

        // look for gz file, only if the original specified path is not a gz.  So part only works to send gzip via content encoding when a non compressed is asked for
        // if you point the the path to gzip you will serve the gzip as content type "application/x-gzip", not text or javascript etc...
        if (!path.endsWith(".gz") && !_fs.exists(path))
        {
            String pathWithGz = path + ".gz";
            if (_fs.exists(pathWithGz))
                path += ".gz";
        }

        File f = _fs.open(path, "r");
        
        if (!f)
            return false;

        if (_cache_header.length() != 0)
            server.sendHeader("Cache-Control", _cache_header);

        server.streamFile(f, contentType);
        return true;
    }

    static String getContentType(const String &path)
    {
        if (path.endsWith(".html"))
            return MIME_TYPE_HTML;
        else if (path.endsWith(".htm"))
            return MIME_TYPE_HTML;
        else if (path.endsWith(".css"))
            return MIME_TYPE_CSS;
        else if (path.endsWith(".txt"))
            return MIME_TYPE_TEXT;
        else if (path.endsWith(".js"))
            return MIME_TYPE_JAVA;
        else if (path.endsWith(".json"))
            return MIME_TYPE_JSON;
        else if (path.endsWith(".png"))
            return MIME_TYPE_PNG;
        else if (path.endsWith(".gif"))
            return MIME_TYPE_GIF;
        else if (path.endsWith(".jpg"))
            return MIME_TYPE_JPG;
        else if (path.endsWith(".ico"))
            return MIME_TYPE_ICO;
        else if (path.endsWith(".svg"))
            return MIME_TYPE_SVG;
        else if (path.endsWith(".ttf"))
            return "application/x-font-ttf";
        else if (path.endsWith(".otf"))
            return "application/x-font-opentype";
        else if (path.endsWith(".woff"))
            return "application/font-woff";
        else if (path.endsWith(".woff2"))
            return "application/font-woff2";
        else if (path.endsWith(".eot"))
            return "application/vnd.ms-fontobject";
        else if (path.endsWith(".sfnt"))
            return "application/font-sfnt";
        else if (path.endsWith(".xml"))
            return MIME_TYPE_XML;
        else if (path.endsWith(".pdf"))
            return "application/pdf";
        else if (path.endsWith(".zip"))
            return "application/zip";
        else if (path.endsWith(".gz"))
            return "application/x-gzip";
        else if (path.endsWith(".appcache"))
            return "text/cache-manifest";
        return MIME_TYPE_DATA;
    }

  protected:
    FS _fs;
    String _uri;
    String _path;
    String _cache_header;
    bool _isFile;
    size_t _baseUriLength;
};

#endif // _IOT_FILE_HANDLER_H

/******************************************************************************/