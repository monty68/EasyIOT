/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software

 SOAPEncoder.h: interface for the SOAPEncoder class.

	$Log: SOAPEncoder.h,v $
	Revision 1.1  2005/03/22 22:22:14  gjpc
	This is the intial check in of the Simple SOAP library.
	
	The code compiles and executes under MSVC .NET and GNU 3.3
	
	It has been run under Debian, SUSE, CYGWIN and WinXP
	
	Revision 1.2  2004/04/23 16:59:26  gjpc
	expanded the Simple SOAP package to allow RPC's within RPC's
	
*/
//////////////////////////////////////////////////////////////////////
/*
#if !defined(SOAPENCODER_H)
    #include "SOAPEncoder.h"
#endif // SOAPENCODER_H
*/

#if !defined(SOAPENCODER_H)
#define SOAPENCODER_H

#ifndef _STRING_
    #include <string>
#endif // _STRING_

#ifndef _INC_TIME
    #include <time.h>
#endif // _INC_TIME

#ifndef _VECTOR_
	#include <vector>
#endif // _VECTOR_

// Forward declaration.
class SOAPMethod;
class SOAPFault;

// Here's something that templates and C++ still can't handle.  Using this
// in conjunction with the encodeArgument() methods below, we avoid
// type WRT the argument names.  Spell it wrong and you get a compile
// time error.  Better at compile-time then run-time :).

#define encodeArg(a) encodeArgument( "" #a "" , a )
#define __int64 long long

class SOAPEncoder  
{
public:

    typedef std::vector<long>		LongArray;
    typedef std::vector<std::string>	StringArray;
    // Constructor/ Destructor
    SOAPEncoder();
    virtual ~SOAPEncoder();
    
    // Encoding functions
    
    // encodeMethodCall
    // Description:
    //  Given a method, turns it into a SOAP message.
    // Arugments:
    //  aMethod: The method to encode.
    // Return Value:
    //  aMethod as a SOAP compliant string.
    virtual std::string encodeMethodCall( SOAPMethod& aMethod );
    
    // encodeMethodResponse
    // Description:
    //  Pretty much the same thing as encodeMethodCall,
    //  except this one sets the aMethod name to the following:
    //  [aMethod.methodName()]Response.  You will see this called
    //  in response to a call to SOAPMethod.execute().
    // Arugments:
    //  aMethod: The method to encode.
    // Return Value:
    //  aMethod as a SOAP compliant string.
    virtual std::string encodeMethodResponse( SOAPMethod& aMethod );
    
    // encodeArgument
    // Description:
    //	This set of overloads encodes the argument using 
    //  the xsd:[type] data as <szArgName>value</szArgName>.
    //  To avoid spelling errors, I recommend using the
    //  encodeArg macro, which can produce the argument name
    //  and value.
    // Arugments:
    //	szArgName: Name of the arguement.
    //  value: Value to encode.
    // Return Value:
    //	Returns a string representing the value when encoded.
    virtual std::string encodeArgument( const std::string& szArg );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const std::string& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const int& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const __int64& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const short& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const char& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const unsigned int& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const unsigned __int64& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const unsigned short& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const unsigned char& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const float& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const double& value );
    virtual std::string encodeArgument( const std::string& szArgName, 
        const bool& value );
    virtual std::string encodeArgument( const std::string& szArgName,
        LongArray& value );
    virtual std::string encodeArgument( const std::string& szArgName,
        StringArray& value );

    // encodeBase64
    // Description:
    //  This one is a bit special.  It takes a pointer and the length
    //  of the data to encode from that pointer in bytes, then base64
    //  encodes the data.
    // Arugments:
    //  szArgName: Name of the argument.
    //  value: Pointer to the start of the bytestream.
    //  ulSizeofValue: Length of the stream, in bytes.
    // Return Value:
    //  Returns a string representing the encoded value.
    virtual std::string encodeBase64  ( const std::string& szArgName, 
        void* value, unsigned long ulSizeofValue );
    
    // clientFaultClass
    // Description:
    //	For this encoder, returns a string that represents the
    //  Client fault class.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string clientFaultClass();
    
    // serverFaultClass
    // Description:
    //	For this encoder, returns a string that represents the
    //  Client fault class.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string serverFaultClass();
    
    // versionMismatchFaultClass
    // Description:
    //	For this encoder, returns a string that represents the
    //  Client fault class.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string versionMismatchFaultClass();
    
    // mustUnderstandFaultClass
    // Description:
    //	For this encoder, returns a string that represents the
    //  Client fault class.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string mustUnderstandFaultClass();
    
    // addBeginTag
    // Description:
    //	For this encoder, adds a begin tag to the stream.  You 
    //  must not provide anything other than the name.
    //  Ex. give the method "startTag", not "<startTag>"
    // Arugments:
    //  szValue: Name of the tag.
    // Return Value:
    //  The string as encoded to this point.
    virtual std::string addBeginTag( const std::string& szValue );
    
    // addEndTag
    // Description:
    //	For this encoder, adds an end tag to the stream.  You 
    //  must not provide anything other than the name.
    //  Ex. give the method "endTag", not "</endTag>"
    // Arugments:
    //  szValue: Name of the tag.
    // Return Value:
    //  The string as encoded to this point.
    virtual std::string addEndTag( const std::string& szValue );
    
    // encodeFault
    // Description:
    //	For this encoder, encodes the SOAPFault.
    //  Must be used apart from other SOAPEncoder methods.
    // Arugments:
    //  soapFault: Name of the tag.
    // Return Value:
    //  Encodes the complete fault into a complete SOAP Envelope.
    virtual std::string encodeFault( SOAPFault& soapFault );

	// encodeFaultContent
	// write the contents of the fault into an ostream
	// we need this to allow embedding of fault return in a 
	// detail description of a container RPC fault
	// Arguments:
	//  soapFault - the RPC's fault object
	//  szSteam   - the Container RPC's detail fault stream
	virtual void encodeFaultContent( SOAPFault& soapFault, std::ostringstream &szStream );

	// encodeMethodResonseContents
	// write the contents of a Method response return to a string
	// this allows the returns from embeded RPC's to be returned
	// into the Container RPC's
	// Argument
	//  aMethod - the Soap Method that will encode it's success response
    virtual std::string encodeMethodResonseContents( SOAPMethod& aMethod );

protected:
    
    // encodeMethod
    // Description:
    //  Called by the encodeMethodResponse
    //  and encodeMethodCall members to finish
    //  encoding the method.  As you may guess, the
    //  two methods share a lot, and this item holds
    //  that common code.
    // Arugments:
    //  aMethod: The method being encoded.
    // Return Value:
    //  Returns the value of the encoded method as 
    //  a valid SOAP string.
    virtual std::string encodeMethod( SOAPMethod& aMethod );
    
    // envelopeOpen
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Envelope element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string envelopeOpen();
    
    // envelopeClose
    // Description:
    //	For this encoder, returns a string that represents the
    //  closing of a Envelope element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string envelopeClose();
    
    // headerOpen
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Header element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string headerOpen();
    
    // headerContents
    // Description:
    //	At this point, this is a place holder.  I haven't
    //  implemented anything that allows the use of header
    //  elements yet.  When I need it, I'll get this one done.
    // Arugments:
    //  N/A
    // Return Value:
    //  Returns the contents of the header field.
    virtual std::string headerContents();
    
    // headerClose
    // Description:
    //	For this encoder, returns a string that represents the
    //  closing of a Header element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string headerClose();
    
    // bodyOpen
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Body element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string bodyOpen();
    
    // bodyClose
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Body element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string bodyClose();
    
    // faultOpen
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Fault element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultOpen();
    
    // faultClose
    // Description:
    //	For this encoder, returns a string that represents the
    //  closing of a Fault element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultClose();
    
    // faultcodeOpen
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Fault:faultcode element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultcodeOpen();
    
    // faultcodeClose
    // Description:
    //	For this encoder, returns a string that represents the
    //  closing of a Fault:faultcode element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultcodeClose();
    
    // faultstringOpen
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Fault:faultstring element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultstringOpen();
    
    // faultstringClose
    // Description:
    //	For this encoder, returns a string that represents the
    //  closing of a Fault:faultstring element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultstringClose();
    
    // faultactorOpen
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Fault:faultactor element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultactorOpen();
    
    // faultactorClose
    // Description:
    //	For this encoder, returns a string that represents the
    //  closing of a Fault:faultactor element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultactorClose();
    
    // faultdetailOpen
    // Description:
    //	For this encoder, returns a string that represents the
    //  opening of a Fault:detail element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultdetailOpen();
    
    // faultdetailClose
    // Description:
    //	For this encoder, returns a string that represents the
    //  closing of a Fault:detail element.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    virtual std::string faultdetailClose();
    
    // isEncodingResponse
    // Description:
    //  Used to indicate if the encoder is currently
    //  closing a SOAP response.
    // Arugments:
    //  N/A
    // Return Value:
    //  true: It is encoding.
    //  false: Is not encoding.
    bool isEncodingResponse();
    
private:
    
    // m_bIsResponse
    // Description: Remembers if this item is encoding
    //  a SOAP response.
    bool m_bIsResponse;
    
    // m_encodedValue
    // Description: Keeps the value of the encoded string
    //  as this object moves itself throughout the encoding process.
    std::string m_encodedValue;
};

#endif // !defined(SOAPENCODER_H)
