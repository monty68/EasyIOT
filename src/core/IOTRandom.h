/*
** EasyIOT - Random Generator Class (TrueRandom - A true random number generator for Arduino.)
**
** This is variant of original work originally implemented as:
** https://code.google.com/archive/p/tinkerit/ https://github.com/Cathedrow/TrueRandom
** Copyright (c) 2010 Peter Knight, Tinker.it! All rights reserved.
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
#ifndef _IOT_RANDOM_H
#define _IOT_RANDOM_H

#include <Arduino.h>
#include <inttypes.h>

#ifndef ICACHE_FLASH_ATTR
#define ICACHE_FLASH_ATTR
#endif

#define IOT_UUID_LENGTH 37
#define IOT_UUID_MAC_INDEX 24

typedef union {
	uint8_t b[16];
	struct
	{
		uint64_t ll1;
		uint64_t ll2;
	};
} iot_uuid_t;

class IOTRandom
{
  public:
	ICACHE_FLASH_ATTR IOTRandom();
	ICACHE_FLASH_ATTR int rand();
	ICACHE_FLASH_ATTR long random();
	ICACHE_FLASH_ATTR long random(long howBig);
	ICACHE_FLASH_ATTR long random(long howsmall, long how);
	ICACHE_FLASH_ATTR int randomBit(void);
	ICACHE_FLASH_ATTR char randomByte(void);
	ICACHE_FLASH_ATTR void memfill(char* location, int size);
	ICACHE_FLASH_ATTR void mac(uint8_t* macLocation);
	ICACHE_FLASH_ATTR void uuid(uint8_t* uuidLocation);

	ICACHE_FLASH_ATTR String uuidToString(iot_uuid_t& u) { return uuidToString(&u.b[0]); }
	ICACHE_FLASH_ATTR String uuidToString(iot_uuid_t* u) { return uuidToString(&u->b[0]); }	
	ICACHE_FLASH_ATTR String uuidToString(uint8_t* uuidLocation);

	ICACHE_FLASH_ATTR String uuidGenerator(bool addMAC = false);
	ICACHE_FLASH_ATTR void uuidGenerator(iot_uuid_t* u, bool addMAC = false);
	ICACHE_FLASH_ATTR void uuidGenerator(iot_uuid_t& u, bool addMAC = false) { uuidGenerator(&u, addMAC); }	
	
    bool useRNG;
  private:
    unsigned long lastYield;
    ICACHE_FLASH_ATTR int randomBitRaw(void);
    ICACHE_FLASH_ATTR int randomBitRaw2(void);
};

// Just in case! Backwards compatability
#ifndef ESP8266TrueRandom
#define ESP8266TrueRandom iotRandom
#endif
extern IOTRandom iotRandom;

#endif // _IOT_RANDOM_H
/******************************************************************************/