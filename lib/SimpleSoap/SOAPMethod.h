/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPMethod.h: interface for the SOAPMethod class.
//
//////////////////////////////////////////////////////////////////////

/*
#if !defined(SOAPMETHOD_H)
    #include "SOAPMethod.h"
#endif // SOAPMETHOD_H
*/

//#include <iostream>
#include <memory>

#if !defined(SOAPMETHOD_H)
#define SOAPMETHOD_H

#ifndef _STRING_
    #include <string>
#endif // _STRING_
#if !defined(SOAPELEMENT_H)
    #include "SOAPElement.h"
#endif // !defined(SOAPELEMENT_H)
#if !defined(SOAPFAULT_H)
    #include "SOAPFault.h"
#endif // !defined(SOAPFAULT_H)

// Forward declaration.
class SOAPEncoder;

class SOAPMethod  
{
public:
    // Constructor/ Destructor
    SOAPMethod();
    virtual ~SOAPMethod();
    
    
    // methodName
    // Description:
    ///  Used to get the name of the method.  This is
    ///  matched up using the SOAP Dispatcher to respond
    ///  to a call.
    // Arugments:
    //  N/A
    /// Return Value:
    ///  Returns the name of the method.
    virtual std::string methodName() = 0;

	// extractMethod
	// Description:
	/// extract the method from the call
	/// Return Value
	///	the method
	virtual bool extractMethod( SOAPElement& theCall, SOAPElement& theMethod );
    
	// methodNameAttributes
	// Description:
	///  Used to get the attributes of the method. Some systems
	///  use namespace declarations to resolve handlers.
    virtual std::string methodNameAttributes();

    // encode
    // Description:
    ///  Transforms the method into something that SOAP
    ///  servers and clients can send.  The encoder
    ///  holds the actual data while the client hands
    ///  data to be entered in.  This makes a whole
    ///  lot more sense in the samples that should have shipped
    ///  with the library.
    // Arugments:
    ///  soapEncoder: Instance of the SOAPEncoder being used
    ///      to, you guessed it, encode the response.
    /// Return Value:
    ///  true: Succeeded in encoding the message.
    ///  false: Failed to encode the message.
    virtual bool encode( SOAPEncoder& soapEncoder ) = 0;
    
    // execute
    // Description:
    ///  Only to be called on the server by the dispatcher,
    ///  this method executes the call and returns 
    ///  true if the call succeeded, false if it failed.
    ///  SOAPMethods should keep any return data in a
    ///  member variable.  The information will be
    ///  returned via a call to encode.
    /// Arugments:
    ///  theCall: A SOAPElement (with possible embedded elements)
    ///      that represents the call.
    /// Return Value:
    ///  true: Call succeeded.
    ///  false: Call failed.
    virtual bool execute( SOAPElement& theCall );
    
    // getFault
    // Description:
    ///	If the call fails, this returns the fault the call generated.
    ///  The actaul SOAPFault won't be created unless setFailed() is
    ///  called.
    // Arugments:
    //  N/A
    /// Return Value:
    ///  Pointer to the fault if it exists.
    virtual SOAPFault* getFault();
    
    // setFailed
    // Description:
    ///  Sets the status of the SOAP call to failed and triggers
    ///  the creation of the SOAPFault.
    // Arugments:
    //  N/A
    // Return Value:
    //  N/A
    void setFailed();
    
    // succeeded
    // Description:
    ///  Tells whether or not the call was successful.
    /// Arugments:
    //  N/A
    // Return Value:
    //  true: Call to execute worked.
    //  false: Call to execute failed.
    bool succeeded();

	// isGenric
	/// provide a run around the method encoding
	virtual bool isGeneric() { return false; };
    
	// mustIUnderstand
	/// use to be a helper but colapsing some of the code we
	/// brought it inside
	virtual bool mustIUnderstand(SOAPElement& theCall);

protected:

    // m_bSucceeded
    // Description: Stores the success value.
    bool m_bSucceeded;
    
    // m_pFault
    // Description: 
	/// Stores a pointer to the fault data (if any).
    std::unique_ptr<SOAPFault> m_pFault;

	// unkownMethodFault
	/// provide a method for Unkown Method name Faults
	void unkownMethodFault( SOAPElement aMethod );

	// wrongNumberArgumentsFault
	/// provide a method for Unkown Method name Faults
	void wrongNumberArgumentsFault( SOAPElement aMethod, const char *num );

	// ImustUnderstand
	/// provide a method to throw the understand fault
	bool ImustUnderstand( SOAPElement aElement );
};

#endif // !defined(SOAPMETHOD_H)
