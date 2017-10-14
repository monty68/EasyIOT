/*
** EasyIOT - Device Function Class
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
#ifndef _IOT_FUNCTION_H
#define _IOT_FUNCTION_H

#include <Arduino.h>
#include "esp_log.h"
#include "IOTProperty.h"
#include "IOTRandom.h"
#include "IOTHttp.h"

/*
** Forward Reference
*/
class IOTMaster;
class IOTFunction;

#define IOTFunction_MAX_TAG 12
#define IOTFunction_MAX_LABEL 48

/*
** State Values
*/
#define IOT_STOPPED 0
#define IOT_RUNNING 1
#define IOT_ERROR 2

/*
** Function Class
*/
class IOTFunction
{
public:
  friend class IOTMaster;
  friend class IOTProperty;

  IOTFunction(const char *tag, uint8_t numProperties);
  ~IOTFunction();

  inline const char * iotTag(void) { return _tag; }
  inline void iotDebug(esp_log_level_t level) { esp_log_level_set(_tag, level); }
  virtual void iotRestart(void){};
  virtual void iotStartup(void) = 0;
  virtual void iotShutdown(void) = 0;
  virtual void iotService(void) = 0;

  IOTFunction *Function(const char * tag);
  IOTMaster *Master(void) const { return _iotMaster; }
  IOTProperty *Property(uint8_t p = 0);
  virtual IOTHTTP *Server(void) const;
  
  const char *getLabel(void) const;
  void getLabel(char *);
  void setLabel(const char *, bool lock = false);

protected:
  IOTMaster *_iotMaster;
  IOTProperty **_Properties;
  uint8_t _propCount;
  uint8_t _state;
  uint16_t _flags;
  const char *_tag;
  char *_label;

  void _initFunction(void);
  char *_loadLabel(const char *key, char *label, size_t max);
  size_t _loadChars(const char *key, char *store, size_t max);
  size_t _saveChars(const char *key, char *store);
  size_t _loadBytes(const char *key, void *store, size_t len);
  size_t _saveBytes(const char *key, void *store, size_t len);
  void _loadProperty(IOTProperty *prop);
  void _saveProperty(IOTProperty *prop);

  void _postUpdate(uint8_t p, bool urgent = false);
  void _postUpdate(IOTProperty *prop, bool urgent = false);
  virtual bool _propUpdate(IOTProperty *prop) { return true; }
  virtual IOTFunction *listHead(void) const;
  virtual IOTFunction *listTail(void) const;  
  inline IOTFunction *listPrev(void) const { return _listPrev; }
  inline IOTFunction *listNext(void) const { return _listNext; }

private:
  uint32_t _nvsHandle;
  IOTFunction *_listPrev;
  IOTFunction *_listNext;
  static IOTProperty *_nullProperty;
};

/*
** Master Base Class
*/
class IOTMaster : public IOTFunction
{
public:
  friend class IOTFunction;
  IOTMaster(const char *ssid = NULL, const char *pass = NULL, uint16_t port = 80, bool lockWifi = false);
  ~IOTMaster();

  static const char *iotVersion(void);  
  static const char *iotBoard(void);
  static const char *iotModel(void);

  //String iotUUID(void) { return String(_uuid); }
  const char *iotUUID(void) const { return &_uuid[0]; }
    
  void iotStartup(void);
  void iotShutdown(void);
  void iotService(void);
  void iotRestart(void);  
  void iotReboot(void) { _needReboot = true; }

  IOTFunction& addFunction(IOTFunction &fun) { (void)addFunction(&fun); return fun; }
  void addFunction(IOTFunction *fun);
  IOTHTTP *Server(void) const;
  
protected:
  void sysReboot(void);
  void sysReset(void);  
  bool _propUpdate(IOTProperty *prop);
  IOTFunction *listHead(void) const;
  IOTFunction *listTail(void) const;
  IOTHTTP *_webServer;

private:
  char _uuid[IOT_UUID_LENGTH + 1];
  bool _needReboot;
  uint64_t _chipID;
  void listHead(IOTFunction *head);
  void listTail(IOTFunction *tail);
  void listInsert(IOTFunction *pBot, IOTFunction *pSibling),
      listAppend(IOTFunction *pBot),
      listRemove(IOTFunction *pBot);       
};

#define IOT IOTMaster

#endif // _IOT_FUNCTION_H
/******************************************************************************/
