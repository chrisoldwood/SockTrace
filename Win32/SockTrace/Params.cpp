/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		PARAMS.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CParams class definition.
**
*******************************************************************************
*/

#include "AppHeaders.hpp"

#ifdef _DEBUG
// For memory leak detection.
#define new DBGCRT_NEW
#endif

/******************************************************************************
** Method:		Constructor.
**
** Description:	.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

CParams::CParams()
{
}

/******************************************************************************
** Method:		Destructor.
**
** Description:	.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

CParams::~CParams()
{
	m_oParams.RemoveAll();
}

/******************************************************************************
** Method:		Set()
**
** Description:	Set the value for a parameter.
**
** Parameters:	strParam	The parameter name.
**				strValue	The parameter value.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CParams::Set(const CString& strParam, const CString& strValue)
{
	if (m_oParams.Exists(strParam))
		m_oParams.Remove(strParam);

	m_oParams.Add(strParam, strValue);
}

/******************************************************************************
** Method:		Find()
**
** Description:	Find the value for a parameter.
**
** Parameters:	strParam	The parameter name.
**
** Returns:		The parameter value or the empty string if not found.
**
*******************************************************************************
*/

CString CParams::Find(const CString& strParam) const
{
	CString strValue;

	m_oParams.Find(strParam, strValue);

	return strValue;
}
