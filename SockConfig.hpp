/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKCONFIG.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CSockConfig class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef SOCKCONFIG_HPP
#define SOCKCONFIG_HPP

/******************************************************************************
** 
** The data class used to store a socket trace configuration.
**
*******************************************************************************
*/

class CSockConfig
{
public:
	//
	// Members.
	//
	CString	m_strType;			// The socket type.
	int		m_nType;			// The socket type (STREAM/DGRAM).
	uint	m_nSrcPort;			// The source port number.
	CString	m_strDstHost;		// The destination hostname.
	CString	m_strDstAddr;		// The destination address.
	uint	m_nDstPort;			// The destination port number.
	CString	m_strSendFile;		// The Send log filename.
	CString	m_strRecvFile;		// The Receive log filename.
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

#endif // SOCKCONFIG_HPP
