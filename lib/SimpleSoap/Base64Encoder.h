/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// Base64Encoder.h: interface for the Base64Encoder class.
//
//////////////////////////////////////////////////////////////////////
/*
#if !defined(BASE64ENCODER_H)
    #include "Base64Encoder.h"
#endif // !defined(BASE64ENCODER_H)
*/
#if !defined(BASE64ENCODER_H)
#define BASE64ENCODER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _STRING_
    #include <string>
#endif // _STRING_

class Base64Encoder  
{
public:
	// Constructor/ Destructor
	Base64Encoder();
	virtual ~Base64Encoder();

	// encodeValue
	// Description:
	//  Takes a series of bytes and base64 encodes them.
	// Arugments:
	//  value: Pointer to the byte stream to encode.
    //  ulSizeofValue: Number of bytes, starting at value, that
    //      need to be encoded.
	// Return Value:
	//  Returns a base64 representation of the data.
    std::string encodeValue( void* value, unsigned long ulSizeofValue );

	// decodeValue
	// Description:
	//  Decodes the value from a base64 string to its original
    //  binary form.
	// Arugments:
	//  value: base64 encoded version of the data.
    //  ulSizeOfValue: On return, indicates how many bytes are
    //      pointed to by the return value.
	// Return Value:
	//  Pointer to the decoded data.  You need to know what to do with
    //  it when it comes back (because I sure don't).
    unsigned char* decodeValue( const std::string& value, unsigned long& ulSizeofValue );

private:

	// m_lookupTable
	// Description: Every 6-bit field that can be mapped to an entry 
    //   in this table.  
    static const unsigned char lookupTable[65];

	// m_lookdownTable
	// Description: Every byt field that can be mapped to an entry 
    //   in this table.  
    static const unsigned char lookdownTable[256];

	// g_KsizeofBase64Buffer
	// Description: Constant to use for building the 
    //  buffer and encoding/decoding it (used by encodeBuffer
    //  and decodeBuffer).
    static const int g_KsizeofBase64Buffer;
};

#endif // !defined(BASE64ENCODER_H)
