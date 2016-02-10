/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKPAIR.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CSockPair class definition.
**
*******************************************************************************
*/

#include "Common.hpp"
#include "SockPair.hpp"
#include "Params.hpp"
#include "SockConfig.hpp"
#include <WCL/StrCvt.hpp>
#include "SockTraceApp.hpp"

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

CSockPair::CSockPair(CSockConfigPtr pConfig, uint nInstance)
	: m_pConfig(pConfig)
	, m_nInstance(nInstance)
	, m_nBytesSent(0)
	, m_nBytesRecv(0)
{
	CParams oParams;

	// Set the common parameters.
	oParams.Set(TXT("port"), CStrCvt::FormatInt(pConfig->m_nSrcPort));
	oParams.Set(TXT("id"),   CStrCvt::FormatInt(m_nInstance));

	oParams.Set(TXT("dir"), TXT("sent"));

	// Create the sent data log file path.
	m_strSendFile = ParseFileName(m_pConfig->m_strSendFile, oParams);

	oParams.Set(TXT("dir"), TXT("recv"));

	// Create the sent data log file path.
	m_strRecvFile = ParseFileName(m_pConfig->m_strRecvFile, oParams);
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

CSockPair::~CSockPair()
{
}

/******************************************************************************
** Method:		ParseFileName()
**
** Description:	Parse the filename replacing the tokens with actual values.
**
** Parameters:	pszFileName		The filename template.
**				oParams			The parameter values.
**
** Returns:		The parsed file name prefixed with the path.
**
*******************************************************************************
*/

CPath CSockPair::ParseFileName(const tchar* pszFileName, const CParams& oParams)
{
	ASSERT(pszFileName != nullptr);

	CPath  strFileName = pszFileName;
	size_t nStartChar  = Core::npos;

	// For all parameters...
	while ((nStartChar = strFileName.Find('%')) != Core::npos)
	{
		size_t nEndChar = Core::npos;

		// Find the parameter end marker.
		if ((nEndChar = strFileName.Find('%', nStartChar+1)) == Core::npos)
			break;

		// Find the parameter value.
		CString strValue = oParams.Find(strFileName.Mid(nStartChar+1, nEndChar-nStartChar-1));

		// Substitute the parameter.
		strFileName.Delete(nStartChar, nEndChar-nStartChar+1);
		strFileName.Insert(nStartChar, strValue);
	}

	return App.m_appDataFolder / strFileName;
}
