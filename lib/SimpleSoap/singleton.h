//////////////////////////////////////////////////////////////////////
// Name:		singleton.h
// Class:	    Singleton
// $Revision: 1.1 $
// $Modtime: 6/02/99 7:14p $
// Description:	Used to make any class a Singleton.
// Usage:		Used to make any class into a Singleton.  A normal class
//              definition should minimally look like this:
/*
class CMyClass : private Singleton<CMyClass>
{
private:
    // Friend so that the constructor can be called from the
    // parent.
    friend Singleton<CMyClass>;
    friend std::auto_ptr<CMyClass>;
    // Private constructor so that users of the class can't
    // instantiate their own copy on the stack.
    CMyClass();

// Destructor
    virtual ~CMyClass();

protected:
    ...
public:

    // Hide the base class so that multiple compilation modules
    // don't try to generate multiple copies of m_ptheSingleton
    static CMyClass& Instance();
    ...
};

In implementation file:

CMyClass& CMyClass::Instance()
{
    return Singleton<CMyClass>::Instance();
}
*/
//              Users of the CMyClass would access members and functions
//              one of two ways:
/* Option 1:
void foo()
{
    CMyClass& myClass = CMyClass::Instance();
    ...
}
*/
/* Option 2:
void foo()
{
    CMyClass::Instance().SomeFunction();
    ...
}
*/
// Patterns:    Implements Singleton from Design Patterns by GOF.
// Author:		Scott Seely 
/*
/*
   $History: singleton.h $
 * 
 * *****************  Version 3  *****************
 * User: Scott        Date: 6/02/99    Time: 7:21p
 * Updated in $/book/workarea/Include
 * Finished changes to code.  I think things are ready to go now.
 * 
 * *****************  Version 2  *****************
 * User: Scott        Date: 6/02/99    Time: 6:58a
 * Updated in $/book/workarea/Include
 * Finished off comments.  Added VSS macros.
*/
//////////////////////////////////////////////////////////////////////
#ifndef SINGLETON_H
#define SINGLETON_H

/*
#ifndef SINGLETON_H
    #include "singleton.h"
#endif //SINGLETON_H
*/

#ifndef _MEMORY_
    #include <memory>
#endif // _MEMORY_

#ifndef _EXCEPTION_
	#include <exception>
#endif // _EXCEPTION_

template <class T>
class Singleton
{
private:
    // Pointer to the one and only instance.
    // As an auto_ptr, we won't get memory leaks
    // because it will be destroyed when the program
    // shuts down and before memory leak detection kicks in.
    static std::auto_ptr<T> m_ptheSingleton;

protected:
    // Constructor
    // Allows derived classes access to the constructor.
    Singleton(){}

    // Allows access to the instance.
    // Derived classes should make this public by naming the method
    // in the derived class's public section.  This gives only one
    // path to one instance of m_ptheSingleton, making the Singleton
    // behavior work.
    static T& Instance();

public:
    // Destructor
    virtual ~Singleton(){}
};

template <class T>
std::auto_ptr<T> Singleton<T>::m_ptheSingleton;

template <class T>
T& Singleton<T>::Instance()
{
    // All auto_ptrs are initalized to NULL.  
    // Only create the object if needed.
    if ( NULL == m_ptheSingleton.get() ) 
    {
        m_ptheSingleton = std::auto_ptr<T>( new T() );
    }
    if ( NULL == m_ptheSingleton.get() )
    {
        throw std::bad_alloc();
    }
    return *m_ptheSingleton;
}

#endif // SINGLETON_H

