/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

*/
// SOAPObjectCreator.h: interface for the SOAPObjectCreator class.
//
//////////////////////////////////////////////////////////////////////
/*
#if !defined(SOAPOBJECTCREATOR_H)
    #include "SOAPObjectCreator.h"
#endif // !defined(SOAPOBJECTCREATOR_H)
*/
#if !defined(SOAPOBJECTCREATOR_H)
#define SOAPOBJECTCREATOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _STRING_
    #include <string>
#endif // _STRING_

class SOAPObject;

class SOAPObjectCreator  
{
public:
    // Constructor/ Destructor
    SOAPObjectCreator();
	SOAPObjectCreator(SOAPObjectCreator& rhs) {}
    virtual ~SOAPObjectCreator();
    
    // newSOAPObject
    // Description:
    //  Creates a new SOAP object that can hopefully do something
    //  worthwhile for us.  
    // Arugments:
    //  N/A
    // Return Value:
    //  Returns a pointer to a newly allocated SOAP object.
    virtual SOAPObject* newSOAPObject() = 0;
    
    // createdObjectName
    // Description: 
    //  Used to tell the subsystem the name of the
    //  object this creator makes.  This is used by 
    //  a server to create the correct object in response
    //  to a SOAP request.
    // Arugments:
    //  N/A
    // Return Value:
    //  Returns the name of the created object.
    virtual std::string createdObjectName() = 0;
};

#endif // !defined(SOAPOBJECTCREATOR_H)
