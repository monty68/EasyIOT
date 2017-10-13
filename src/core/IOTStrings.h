/*
** EasyIOT - Common String Constants
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
#ifndef _IOT_STRINGS_H
#define _IOT_STRINGS_H

extern const char *nvs_errors[];
extern const char *strNull;

extern const char *strClosed;
extern const char *strEasyIoT;
extern const char *strFalse;
extern const char *strHigh;
extern const char *strLabel;
extern const char *strLow;
extern const char *strOff;
extern const char *strOn;
extern const char *strOpen;

extern const char *strPrimary;
extern const char *strSecondary;
extern const char *strSystem;

extern const char *strTime;
extern const char *strTimeServer;
extern const char *strTimeZone;
extern const char *strTrue;

extern const char *strValue;
extern const char *strValve;


#ifndef nvs_error
#define nvs_error(e) (((e) > ESP_ERR_NVS_BASE) ? nvs_errors[(e) & ~(ESP_ERR_NVS_BASE)] : nvs_errors[0])
#endif

#endif // _IOT_STRINGS_H

/******************************************************************************/