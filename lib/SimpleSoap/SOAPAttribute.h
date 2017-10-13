/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPAttribute.h: interface for the SOAPAttribute class.
//
//////////////////////////////////////////////////////////////////////
/*
#if !defined(SOAPATTRIBUTE_H)
    #include "SOAPAttribute.h"
#endif // !defined(SOAPATTRIBUTE_H)
*/
#if !defined(SOAPATTRIBUTE_H)
#define SOAPATTRIBUTE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _STRING_
    #include <string>
#endif // _STRING_

class SOAPAttribute  
{
public:
    SOAPAttribute();
    virtual ~SOAPAttribute();
    
    // namespaceName
    // Description:
    //  Returns a reference to the internal namespace name.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    std::string& namespaceName();
    
    // accessor
    // Description:
    //  Returns a reference to the internal accessor.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    std::string& accessor();
    
    // value
    // Description:
    //  Returns a reference to the internal value.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    std::string& value();
private:
    
    // m_szNamespaceName
    // Description: Stores the name of the namespace associated 
    // with the attribute.
    std::string m_szNamespaceName;
    
    // m_szAccessor
    // Description: Stores the name of the attributes accessor.
    std::string m_szAccessor;
    
    // m_szValue
    // Description: Value of the attribute.
    std::string m_szValue;
};

#endif // !defined(SOAPATTRIBUTE_H)
