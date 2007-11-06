/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		UDPSOCKPAIR.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CUDPSockPair class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef UDPSOCKPAIR_HPP
#define UDPSOCKPAIR_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include "SockPair.hpp"
#include <NCL/UDPSvrSocket.hpp>

/******************************************************************************
** 
** The pair of sockets used to proxy a UDP client/server connection.
**
*******************************************************************************
*/

class CUDPSockPair : public CSockPair
{
public:
	//
	// Constructors/Destructor.
	//
	CUDPSockPair(CSockConfig* pConfig, uint nInstance);
	virtual ~CUDPSockPair();
	
	//
	// Members.
	//
	CUDPSvrSocket	m_oInpSocket;	// The client connection.
	CUDPSvrSocket	m_oOutSocket;	// The server connection.
	in_addr			m_oSrcAddr;		// Client send address.
	uint			m_nSrcPort;		// Client send port.
	in_addr			m_oDstAddr;		// Server address.
	uint			m_nDstPort;		// Server port.
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

inline CUDPSockPair::CUDPSockPair(CSockConfig* pConfig, uint nInstance)
	: CSockPair(pConfig, nInstance)
{
	// Reset source address and port.
	m_oSrcAddr.s_addr = INADDR_NONE;
	m_nSrcPort        = 0;
}

inline CUDPSockPair::~CUDPSockPair()
{
}

#endif // UDPSOCKPAIR_HPP
