/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		TCPSOCKPAIR.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CTCPSockPair class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef TCPSOCKPAIR_HPP
#define TCPSOCKPAIR_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include "SockPair.hpp"
#include <NCL/TCPCltSocket.hpp>

// Forward declarations.
class CSockConfig;

/******************************************************************************
** 
** The pair of sockets used to proxy a TCP client/server connection.
**
*******************************************************************************
*/

class CTCPSockPair : public CSockPair
{
public:
	//
	// Constructors/Destructor.
	//
	CTCPSockPair(CSockConfigPtr pConfig, uint nInstance, CTCPCltSocketPtr pInpSocket, CTCPCltSocketPtr pOutSocket);
	virtual ~CTCPSockPair();
	
	//
	// Members.
	//
	CTCPCltSocketPtr	m_pInpSocket;	// The client connection.
	CTCPCltSocketPtr	m_pOutSocket;	// The server connection.
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

inline CTCPSockPair::CTCPSockPair(CSockConfigPtr pConfig, uint nInstance, CTCPCltSocketPtr pInpSocket, CTCPCltSocketPtr pOutSocket)
	: CSockPair(pConfig, nInstance)
	, m_pInpSocket(pInpSocket)
	, m_pOutSocket(pOutSocket)
{
}

inline CTCPSockPair::~CTCPSockPair()
{
}

#endif // TCPSOCKPAIR_HPP
