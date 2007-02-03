/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKPAIR.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CSockPair class definition.
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

CSockPair::CSockPair(CSockConfig* pConfig, uint nInstance)
	: m_pConfig(pConfig)
	, m_nInstance(nInstance)
	, m_nBytesSent(0)
	, m_nBytesRecv(0)
{
	ASSERT(pConfig != NULL);

	CParams oParams;

	// Set the common parameters.
	oParams.Set("port", CStrCvt::FormatInt(pConfig->m_nSrcPort));
	oParams.Set("id",   CStrCvt::FormatInt(m_nInstance));

	oParams.Set("dir", "sent");

	// Create the sent data log file path.
	m_strSendFile = ParseFileName(m_pConfig->m_strSendFile, oParams);

	oParams.Set("dir", "recv");

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

CPath CSockPair::ParseFileName(const char* pszFileName, const CParams& oParams)
{
	ASSERT(pszFileName != NULL);

	CPath strFileName = pszFileName;
	int   nStartChar  = -1;

	// For all parameters...
	while ((nStartChar = strFileName.Find('%')) != -1)
	{
		int nEndChar = -1;

		// Find the parameter end marker.
		if ((nEndChar = strFileName.Find('%', nStartChar+1)) == -1)
			break;

		// Find the parameter value.
		CString strValue = oParams.Find(strFileName.Mid(nStartChar+1, nEndChar-nStartChar-1));

		// Substitute the parameter.
		strFileName.Delete(nStartChar, nEndChar-nStartChar+1);
		strFileName.Insert(nStartChar, strValue);
	}

	return CPath::ApplicationDir() / strFileName;
}
