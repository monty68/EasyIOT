/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPElement.h: interface for the SOAPElement class.
//
//////////////////////////////////////////////////////////////////////

/*
#if !defined(SOAPELEMENT_H)
    #include "SOAPElement.h"
#endif // !defined(SOAPELEMENT_H)
*/
#if !defined(SOAPELEMENT_H)
#define SOAPELEMENT_H

#ifndef _STRING_
    #include <string>
#endif // _STRING_

#ifndef _VECTOR_
    #include <vector>
#endif // _VECTOR_

#if !defined(SOAPATTRIBUTE_H)
    #include "SOAPAttribute.h"
#endif // !defined(SOAPATTRIBUTE_H)

#include <sstream>

class SOAPElement  
{
public:
    // Constructor/ Destructor
    SOAPElement();
    SOAPElement( const SOAPElement& rhs );
    SOAPElement& operator=( const SOAPElement& rhs );
    virtual ~SOAPElement();
    
    // accessorName
    // Description:
    //  Returns the string used to access the SOAPElement.
    // Arugments:
    //  N/A
    // Return Value:
    //  Reference to the actual accessor name.  Since I have no 
    //  intention of guarding the getting and setting of the accessor
    //  name, I return a reference to the internal element.
    std::string& accessorName();
    
    // namespaceName
    // Description:
    //  Returns the string used as the SOAPElement namespace.
    // Arugments:
    //  N/A
    // Return Value:
    //  Reference to the actual namespace name.  Since I have no 
    //  intention of guarding the getting and setting of the 
    //  namespace name, I return a reference to the internal element.
    std::string& namespaceName();
    
    // value
    // Description:
    //  Gets the value inside the element (between the 
    //  begin and end tags).  It's up to you to decide
    //  how to interpret it.
    // Arugments:
    //  N/A	
    // Return Value:
    //  Reference to the actual value.  Since I have no intention
    //  of guarding the getting and setting of the value,
    //  I return a reference to the internal element.
    std::string& value();
    
    // addAttribute
    // Description:
    //	Adds an attribute to the element.
    // Arugments:
    //  theAttribute: The attribute to add.
    // Return Value:
    //  true: Successfully added.
    //  false: Didn't add.
    bool addAttribute( SOAPAttribute theAttribute );
    
    // getAttribute
    // Description:
    //  Given an attribute name, this finds the named attribute.
    //  If you are looking for a fully qualified attribute (namespace
    //  plus base name), just pass it in.  I'll find the colon (':').
    // Arugments:
    //  szAttributeName: Name of the attribute to find.
    //  szValue: On return, contains a copy of the internal 
    //  attribute.
    // Return Value:
    //  true: Found it.
    //  false: Didn't find it.
    bool getAttribute( const std::string& szAttributeName, 
        SOAPAttribute& szValue );
    
    // addElement
    // Description:
    //  Adds an element beneath the current element.
    // Arugments:
    //  pElement: Pointer to the element.  This object
    //      expects to assume ownership of the element and
    //      will destroy it when this element is deleted or
    //      goes out of scope.  Don't call this method if you
    //      want to control the objects lifespan.
    // Return Value:
    //  true: Successfully added.
    //  false: Didn't add.
    bool addElement( SOAPElement* pElement );
    
    // getElement
    // Description:
    //  Given an element name, this finds the named element.
    //  If you are looking for a fully qualified element (namespace
    //  plus base name), just pass it in.  I'll find the colon (':').
    // Arugments:
    //  szElementName: Name of the element to find.
    //  pValue: Pointer to a SOAPElement pointer.  You don't need to
    //      delete this pointer when you are done-- this class still
    //      owns the memory.
    // Return Value:
    //  true: Found it.
    //  false: Didn't find it.
    bool getElement( const std::string& szElementName, 
        SOAPElement** pValue );
    
    // numElements
    // Description:
    //  Tells the caller how many immediate subelements
    //  are contained by this SOAPElement.  This does
    //  not return the total number of nodes in the tree.
    // Arugments:
    //  N/A
    // Return Value:
    //  Number of immediate child nodes.
    long numElements() const;
    
    // numAttributes
    // Description:
    //  Tells the caller how many attributes this element contains.
    // Arugments:
    //  N/A
    // Return Value:
    //  See description.
    long numAttributes();
    
    // elementAt
    // Description:
    //  Returns the SOAPElement at a specific index.
    // Arugments:
    //  index: Zero-based index of the SOAPElement you
    //      want to retrieve.
    // Return Value:
    //  Reference to the requested SOAPElement.
    SOAPElement& elementAt( long index );
    
    // attributeAt
    // Description:
    //  Returns the SOAPAttribute at a specific index.
    // Arugments:
    //  index: Zero-based index of the SOAPAttribute you
    //      want to retrieve.
    // Return Value:
    //  Reference to the requested SOAPAttribute.
    SOAPAttribute& attributeAt( long index );
    

	void serialize(std::ostringstream& stream);
	void serialize(std::ostringstream& stream, SOAPElement& element);

private:
    
    // m_szAccessorName
    // Description: Name of the element (minus namespace name).
    std::string m_szAccessorName;
    
    // m_szNamespace
    // Description: Name of the namespace (minus accessor name).
    std::string m_szNamespace;
    
    // m_szValue
    // Description: Value contained between the start and stop tags.
    std::string m_szValue;
    
    // Retrieve attributess using attribute name.
    typedef std::vector< SOAPAttribute > AttributeContainer;
    
    // m_attributes
    // Description: List of attributes.
    AttributeContainer m_attributes;
    
    // ElementContainer
    // Description: Container of SOAPElements.
    //typedef std::vector< std::auto_ptr<SOAPElement> > ElementContainer;
	typedef std::vector<SOAPElement*> ElementContainer;

    // m_internalElements
    // Description: The SOAPElements owned by this SOAPElement.
    ElementContainer m_internalElements;
};

#endif // !defined(SOAPELEMENT_H)
