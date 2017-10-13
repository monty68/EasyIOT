/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPParser.h: interface for the SOAPParser class.
// Description: This class takes an unparsed XML document
//    and parses it up into it's constituent nodes, attributes, and 
//    values.  It does some things that are specific to SOAP,
//    but for the most part it could probably parse up any XML doc.
//
//////////////////////////////////////////////////////////////////////
/*
#if !defined(SOAPPARSER_H)
    #include "SOAPParser.h"
#endif // !defined(SOAPPARSER_H)
*/
#if !defined(SOAPPARSER_H)
#define SOAPPARSER_H

#include <memory>
#ifndef _STRING_
    #include <string>
#endif // _STRING_

#ifndef _MAP_
    #include <map>
#endif // _MAP_
    
#if !defined(SOAPFAULT_H)
    #include "SOAPFault.h"
#endif // !defined(SOAPFAULT_H)

class SOAPElement;

class SOAPParser  
{
public:
    // Constructor/ Destructor
    SOAPParser();
    virtual ~SOAPParser();
    
    // Maps a namespace identifier to a URN.
    typedef std::map<std::string, std::string> XMLNStoURN;
    
    // parseMessage
    // Description:
    //  Given a SOAP string, parses it into new elements and 
    //  deposits them into the root SOAPElement passed in via
    //  soapMessage.
    // Arugments:
    //  szMessage: The message to parse.
    //  soapMessage: The root SOAPElement to fill in.
    // Return Value:
    //  true: Successful parse.
    //  false: Unsuccessful parse.
    virtual bool parseMessage( const std::string& szMessage, 
        SOAPElement& soapMessage );
    
    // getNamespacesInUse
    // Description:
    //  Returns the names and URNs of any and all namespaces found
    //  when parsing the message.  If duplicate namespace names are
    //  used with different URNs, only the last one found will 
    //  appear in the set of return values.
    // Arugments:
    //	N/A
    // Return Value:
    //  Returns a reference to the internal namespace to URN mapping.
    XMLNStoURN& getNamespacesInUse();
    
    // getFault
    // Description:
    //	If the call fails, this returns the fault the call generated.
    //  The actaul SOAPFault won't be created unless setFailed() is
    //  called.
    // Arugments:
    //  N/A
    // Return Value:
    //  Pointer to the fault if it exists.
    SOAPFault* getFault();

protected:
    
    // parseMessage
    // Description:
    //  Recursive version of the public interface.  The caller 
    //  doesn't need to know that we pass the current position 
    //  within the message string as we parse, but it doesn't
    //  hurt that the object knows about this.
    // Arugments:
    //  szMesage: Message to parse.
    //  soapElement: Root element to fill in (might be one of the
    //  original's sub-elements).
    //  nCurrentPos: Current position within szMessage.
    // Return Value:
    //  true: Successful parse.
    //  false: Unsuccessful parse.
    virtual bool parseMessage( const std::string& szMessage, 
        SOAPElement& soapElement, long& nCurrentPos );
    
    // setFailed
    // Description:
    //  Sets the status of the SOAP call to failed and triggers
    //  the creation of the SOAPFault.
    // Arugments:
    //  N/A
    // Return Value:
    //  N/A
    void setFailed();
    
    // extractQuotedString
    // Description:
    //  At a given position within a string, find the string with
    //  the matching quote types.
    // Arugments:
    //  szString: String to find a "quoted string" within,
    //  nPos: Position to start at on entrance.  Position after last
    //  quote on exit.
    // Return Value:
    //  String minus the quotes used to surround the quoted string.
    std::string extractQuotedString( const std::string& szString, 
        long& nPos );
    
    // splitNSAndAccessor
    // Description:
    //  Given a "full string" splits the namespace from the rest of 
    //  the string.
    // Arugments:
    //  szFullString: String with namespace and accessor.
    //  szNamespace: On return, has the namespace name filled in 
    //      (if present).
    //  szOther: On return contains the name of the accessor.
    // Return Value:
    //  N/A
    void splitNSAndAccessor( std::string szFullString, 
        std::string& szNamespace, std::string& szOther );
    
    // extractValue  
    // Description:
    //  Extracts the first value it finds in szMessage starting
    //  at nCurrentPos and puts it into theElement.  This stops
    //  processing when it hits a "<".
    // Arugments:
    //	theElement: The element to put the value into.
    //  szMessage: The message to parse.
    //  nCurrentPos: The message to start extracting the value at.
    // Return Value:
    //  true: Extracted the value successfully.
    //  false: Failed to extract the value.
    bool extractValue( SOAPElement& theElement, 
        const std::string& szMessage, long& nCurrentPos );
    
    // extractAttributes
    // Description:
    //  Given an XML start tag, extracts any attributes and their 
    //  values.  Remove any namespace declarations before calling
    //  this method.
    // Arugments:
    //	theElement: The element to add the attributes to.
    //  szBeginTag: The tag to parse.
    // Return Value:
    //  true: Extracted the attributes successfully.
    //  false: Failed to extract the attributes.
    bool extractAttributes( SOAPElement& theElement, 
        std::string szBeginTag );
    
    // extractNamespaces
    // Description:
    //  Extracts any and all namespace names.  Puts the namespace
    //  namesand their associated URNs into the internal namespace
    //  to URN map.
    // Arugments:
    //	szCompleteAccessor: A begin tag.  On return, all namespace
    //      declarations are removed.
    // Return Value:
    //  true: Extracted the namespaces successfully.
    //  false: Failed to extract the namespaces.
    bool extractNamespaces( std::string& szCompleteAccessor );
    
    // extractBeginTag
    // Description:
    //	Extracts the "begin tag".  If the tag is really an end tag,
    //  it indicates this via bIsEndTag.  This also returns the
    //	namespace declared with the element.
    // Arugments:
    //	szBeginTag: On entrance, contains nothing.  On exit, contains
    //      the begin tag.
    //  szNamespace: Contains the namespace of the accessor on exit.
    //      If no namespace is used, this is an empty string.
    //	szAccessorName: Contains the name of the accessor on exit.
    //	szMessage: Contains the message to dig through.
    //	nCurrentPos: On entrance, contains the position to start
    //      parsing.  On exit, contains the spot where the tag
    //      closed.
    //  bIsEmptyTag: Indicates if the tag is a begin AND an end tag.
    //      Ex.:  <anEmptyTag value="myData" />
    //  bIsEndTag: Indicates if the current tag is an end tag.  
    //      Ex.: </anEndTag>
    // Return Value:
    //  true: Extracted the required information successfully.
    //  false: Failed to extract the required information.
    bool extractBeginTag(
        std::string& szBeginTag, 
        std::string& szNamespace, 
        std::string& szAccessorName, 
        std::string szMessage, 
        long &nCurrentPos, 
        bool &bIsEmptyTag, 
        bool &bIsEndTag);
    
    // Data members
    
    // m_namespaceMap
    // Description: Contains the map of namespace name to URN. 
    XMLNStoURN m_namespaceMap;
    
    // m_pFault
    // Description: Stores a pointer to the fault data (if any).
    std::auto_ptr<SOAPFault> m_pFault;
};

#endif // !defined(SOAPPARSER_H)
