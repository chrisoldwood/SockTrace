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
	CTCPSockPair(CSockConfig* pConfig, uint nInstance, CTCPCltSocket* pInpSocket, CTCPCltSocket* pOutSocket);
	virtual ~CTCPSockPair();
	
	//
	// Members.
	//
	CTCPCltSocket*	m_pInpSocket;	// The client connection.
	CTCPCltSocket*	m_pOutSocket;	// The server connection.
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

inline CTCPSockPair::CTCPSockPair(CSockConfig* pConfig, uint nInstance, CTCPCltSocket* pInpSocket, CTCPCltSocket* pOutSocket)
	: CSockPair(pConfig, nInstance)
	, m_pInpSocket(pInpSocket)
	, m_pOutSocket(pOutSocket)
{
}

inline CTCPSockPair::~CTCPSockPair()
{
	delete m_pInpSocket;
	delete m_pOutSocket;
}

#endif // TCPSOCKPAIR_HPP
