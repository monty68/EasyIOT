/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com


This library is public domain software
*/
// SOAPDispatcher.h: interface for the SOAPDispatcher class.
//
//	$Log: SOAPDispatcher.h,v $
//	Revision 1.1  2005/03/22 22:22:14  gjpc
//	This is the intial check in of the Simple SOAP library.
//	
//	The code compiles and executes under MSVC .NET and GNU 3.3
//	
//	It has been run under Debian, SUSE, CYGWIN and WinXP
//	
//	Revision 1.2  2004/04/23 16:59:26  gjpc
//	expanded the Simple SOAP package to allow RPC's within RPC's
//	
//
//////////////////////////////////////////////////////////////////////
/*
#if !defined(SOAPDISPATCHER_H)
    #include "SOAPDispatcher.h"
#endif // !defined(SOAPDISPATCHER_H)
*/
#if !defined(SOAPDISPATCHER_H)
#define SOAPDISPATCHER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _STRING_
    #include <string>
#endif // _STRING_
#if !defined(SOAPELEMENT_H)
    #include "SOAPElement.h"
#endif // !defined(SOAPELEMENT_H)
#ifndef _MAP_
    #include <map>
#endif // _MAP_
#ifndef SINGLETON_H
    #include "singleton.h"
#endif //SINGLETON_H
#ifndef _MEMORY_
    #include <memory>
#endif // _MEMORY_
#if !defined(SOAPOBJECTCREATOR_H)
    #include "SOAPObjectCreator.h"
#endif // !defined(SOAPOBJECTCREATOR_H)

#include "SOAPObject.h"

class SOAPDispatcher : private Singleton< SOAPDispatcher >
{
public:
    // Instance
    // Description:
    //  Returns the single instance of this object within the
    //  program.
    // Arugments:
    //  N/A
    // Return Value:
    //  Reference to the SOAPDispatcher instance.
    static SOAPDispatcher& Instance();
    
    // processMessage
    // Description:
    //  Given a SOAPElement, object name, and method name, this calls
    //  the appropriate SOAPMethod on the appropriate SOAPObject.
    //  If this fails, ask for a SOAPFault.
    // Arugments:
    //  KszObjectName: Name of the SOAPObject 
    //      (SOAPObjectCreator.createdObjectName()).
    //  KszMethodName: Name of the SOAPMethod 
    //      (SOAPMethod.methodName()).
    //  theCall: Pre-parsed message.
    //  bContainsFault: Indicates if the returned string
    //      contains a fault.
    // Return Value:
    //  Returns a SOAP. send-ready response.
    std::string processMessage( const std::string& KszObjectName, 
        const std::string& KszMethodName,
        SOAPElement& theCall,
        bool& bContainsFault );
    
    // registerObject
    // Description:
    //  Adds an object creator to the list of objects the dispatcher
    //  can call.
    // Arugments:
    //  pCreator: Pointer to an allocated SOAPObjectCreator.  The
    //      object assumes ownership of the object and will destroy
    //      it at program termination.
    // Return Value:
    //  true: Object successfully registered.
    //  false: Object already in registry.  You need to delete
    //      the memory yourself in this case (the dispatcher didn't
    //      take ownership of the object).
    bool registerObject( SOAPObjectCreator* pCreator );
    
	// used while within a dispatch to allow fellow methods to 
	// reference each other
	SOAPMethod *GetMethod( const char* methodName );

private:
    // Allow these two to create and destroy this class.
    // The file singleton.h explains why this is necessary.
    friend class Singleton< SOAPDispatcher >;
    friend class std::unique_ptr<SOAPDispatcher>;
    // Constructor/ Destructor
    SOAPDispatcher();
    virtual ~SOAPDispatcher();
    
    // CreatorContainer
    // Description: Makes typing easier for the container.
    typedef std::map<std::string, SOAPObjectCreator*> 
        CreatorContainer;
    
    // m_creatorContainer
    // Description: The dispatcher's registry.
    CreatorContainer m_creatorContainer;

	// hold the curretn list
	SOAPObject::MethodList *currentMethodList;
};

#endif // !defined(SOAPDISPATCHER_H)
