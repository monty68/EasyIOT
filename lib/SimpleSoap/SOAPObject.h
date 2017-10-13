/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPObject.h: interface for the SOAPObject class.
//
//////////////////////////////////////////////////////////////////////

/*
#if !defined(SOAPOBJECT_H)
    #include "SOAPObject.h"
#endif // !defined(SOAPOBJECT_H)
*/

#if !defined(SOAPOBJECT_H)
#define SOAPOBJECT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if !defined(SOAPMETHOD_H)
    #include "SOAPMethod.h"
#endif // SOAPMETHOD_H

#ifndef _LIST_
    #include <list>
#endif // _LIST_

class SOAPObject  
{
public:
    // Constructor/ Destructor
    SOAPObject();
    virtual ~SOAPObject();
    
    // Contains an array of soap methods.
    typedef std::list< SOAPMethod* > MethodList;
    
    // getMethodList
    // Description:
    //  Returns the SOAPMethods that this object implements.
    //  This is a member of the base class because
    //  this information should be constant across implementations.
    // Arugments:
    //  N/A
    // Return Value:
    //  Returns a reference to the m_methodList parameter.
    MethodList& getMethodList();
    
protected:
    
    // insertMethod
    // Description:
    //  Meant to be called by classes derived from SOAPObject.
    //  This method inserts a method into the method list.
    // Arugments:
    //  pMethod: Pointer to one of the SOAPMethods contained
    //      by the SOAPObject.
    // Return Value:
    //  N/A
    void insertMethod( SOAPMethod* pMethod );
    
    // m_methodList
    // Description: List of all the methods implemented
    //  by the derived object.
    MethodList m_methodList;
};

#endif // !defined(SOAPOBJECT_H)
