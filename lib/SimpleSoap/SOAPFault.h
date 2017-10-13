/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPFault.h: interface for the SOAPFault class.
//
//////////////////////////////////////////////////////////////////////
/*
#if !defined(SOAPFAULT_H)
    #include "SOAPFault.h"
#endif // !defined(SOAPFAULT_H)
*/

#if !defined(SOAPFAULT_H)
#define SOAPFAULT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SOAPElement.h"

class SOAPFault : public SOAPElement  
{
public:
    // Constructor/ Destructor
    SOAPFault();
    virtual ~SOAPFault();
    
    
    // getFaultCode
    // Description:
    //  Gets the general reason why the call failed.
    // Arugments:
    //  N/A
    // Return Value:
    //  Returns a string combining the generic fault and
    //  any more specific fault data.
    std::string getFaultCode();
    enum FaultCode { Client, Server, MustUnderstand, VersionMismatch };
    
    // setFaultCode
    // Description:
    //  Sets the generic fault code.
    // Arugments:
    //  faultCode: One of the values found in the FaultCode enumeration.
    // Return Value:
    //  N/A  
    void setFaultCode( FaultCode faultCode );
    
    // setSpecificFault
    // Description:
    //  Allows the caller to set the generic and specific
    //  reasons for the failure of a call.
    // Arugments:
    //  szSpecificFault: The more descriptive reason behind the
    //      failure.
    //  faultCode: One of the values found in the FaultCode enumeration.
    // Return Value:
    //  N/A  
    void setSpecificFault( const std::string& szSpecificFault, FaultCode faultCode = Client );
    
    // faultString
    // Description:
    //  Allows the user to set/get the complete fault string.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    std::string& faultString();
    
    // faultActor
    // Description:
    //  Allows the user to set/get the fault actor.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    std::string& faultActor();
    
    // detail
    // Description:
    //  Allows the user to set/get the fault detail.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    std::string& detail();
    
private:
    
    // m_faultCode
    // Description: Generic reason the call failed.
    FaultCode m_faultCode;
    
    // m_szSpecificFault
    // Description: Specific reason the call failed.
    std::string m_szSpecificFault;
    
    // m_szFaultString
    // Description: Description of the fault.
    std::string m_szFaultString;
    
    // m_szFaultActor
    // Description: Intermediary that caught the fault.
    std::string m_szFaultActor;
    
    // m_szDetail
    // Description: Details behind why the fault occurred.
    std::string m_szDetail;
};

#endif // !defined(SOAPFAULT_H)
