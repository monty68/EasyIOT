/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
#ifndef TSOAPOBJECTCREATOR_H
    #include "TSOAPObjectCreator.h"
#endif // TSOAPOBJECTCREATOR_H
*/

#ifndef TSOAPOBJECTCREATOR_H
#define TSOAPOBJECTCREATOR_H

#if !defined(SOAPOBJECTCREATOR_H)
    #include "SOAPObjectCreator.h"
#endif // !defined(SOAPOBJECTCREATOR_H)

template <class T>
class TSOAPObjectCreator : public SOAPObjectCreator
{
public:
    TSOAPObjectCreator( std::string szObjectName ) :
        m_szObjectName( szObjectName ) {}
    virtual SOAPObject* newSOAPObject() { return new T; }
    virtual std::string createdObjectName() 
        { return m_szObjectName; }

private:
    std::string m_szObjectName;
};


#endif // TSOAPOBJECTCREATOR_H
