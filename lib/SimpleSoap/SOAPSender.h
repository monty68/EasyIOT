// SOAPonProtocol.h: interface for the SOAPonProtocol class.
//
//////////////////////////////////////////////////////////////////////
/*
#if !defined(SOAPonProtocol_H)
    #include "SOAPonProtocol.h"    
#endif // !defined(SOAPonProtocol_H)
*/

#if !defined(SOAPonProtocol_H)
#define SOAPonProtocol_H

//#ifndef _STRING_
    #include <string>
//#endif // _STRING_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class SOAPonProtocol  
{
public:
	SOAPonProtocol();
	virtual ~SOAPonProtocol();
    std::string createSendString( const& std::string szSOAPMessage, const& std::string szObjectName ) = 0;
};

#endif // !defined(SOAPonProtocol_H)
