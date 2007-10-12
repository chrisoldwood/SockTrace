/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		PARAMS.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CParams class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef PARAMS_HPP
#define PARAMS_HPP

/******************************************************************************
** 
** The collection of parameters used when parsing a log filename.
**
*******************************************************************************
*/

class CParams
{
public:
	//
	// Constructors/Destructor.
	//
	CParams();
	~CParams();

	//
	// Methods.
	//
	void    Set (const CString& strParam, const CString& strValue);
	CString Find(const CString& strParam) const;
	
protected:
	// Template shorthands.
	typedef std::map<CString, CString> CStrStrMap;

	//
	// Members.
	//
	CStrStrMap	m_oParams;
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

#endif // PARAMS_HPP
