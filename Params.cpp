/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		PARAMS.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CParams class definition.
**
*******************************************************************************
*/

#include "Common.hpp"
#include "Params.hpp"

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
	m_oParams.clear();
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
	m_oParams[strParam] = strValue;
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
	return m_oParams.find(strParam)->second;
}
