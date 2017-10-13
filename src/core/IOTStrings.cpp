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
#include "IOTStrings.h"

const char *nvs_errors[] = {
    "OTHER", "NOT_INITIALIZED", "NOT_FOUND", "TYPE_MISMATCH", "READ_ONLY", 
    "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE", "REMOVE_FAILED", 
    "KEY_TOO_LONG", "PAGE_FULL", "INVALID_STATE", "INVALID_LENGHT"
};

const char *strNull = "";
const char *strEasyIoT = "EasyIoT";
const char *strTrue = "True";
const char *strFalse = "False";
const char *strOn = "On";
const char *strOff = "Off";
const char *strOpen = "Open";
const char *strClosed = "Closed";
const char *strHigh = "High";
const char *strLow = "Low";
const char *strLabel = "Label";
const char *strPrimary = "Primary";
const char *strSecondary = "Secondary";
const char *strSystem = "System";
const char *strTime = "Time";
const char *strTimeServer = "Time Server";
const char *strTimeZone = "Time Zone";
const char *strValue = "Value";
const char *strValve = "Valve";


/*
const char *IOTFunction::IOTPropertyBase::_strPrefix[] = {
    "Generic Value",
    "Switch",
    "Ambient Temperature",
    "Relative Humidity",
    "Illuminance",
    "Barometer",
    "Pressure",
    "Frequency",
    "Voltage",
    "Current"};

    const char *IOTFunction::IOTPropertyBase::_strSuffix[] = {
    strNull,
    "°C",
    "%RH",
    "Lx",
    "Pa",
    "Hz",
    "V",
    "A"};
*/
/******************************************************************************/