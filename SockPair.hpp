/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKPAIR.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CSockPair class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef SOCKPAIR_HPP
#define SOCKPAIR_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/Path.hpp>

// Forward declarations.
class CSockConfig;
class CParams;

typedef Core::SharedPtr<CSockConfig> CSockConfigPtr;

/******************************************************************************
** 
** The base class for a pair of sockets.
**
*******************************************************************************
*/

class CSockPair
{
public:
	//
	// Constructors/Destructor.
	//
	CSockPair(CSockConfigPtr pConfig, uint nInstance);
	virtual ~CSockPair();
	
	//
	// Members.
	//
	uint			m_nInstance;	// The pair instance ID.
	CSockConfigPtr	m_pConfig;		// The socket config.
	CPath			m_strSendFile;	// Path of log file for data sent.
	CPath			m_strRecvFile;	// Path of log file for data recieved.
	uint			m_nBytesSent;	// Total bytes sent.
	uint			m_nBytesRecv;	// Total bytes recieved.

private:
	//
	// Internal methods.
	//
	CPath ParseFileName(const tchar* pszFileName, const CParams& oParams); 
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

#endif // SOCKPAIR_HPP
