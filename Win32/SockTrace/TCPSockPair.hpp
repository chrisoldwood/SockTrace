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

class CTCPSockPair
{
public:
	//
	// Constructors/Destructor.
	//
	CTCPSockPair(uint nInstance, CSockConfig* pConfig, CTCPCltSocket* pInpSocket, CTCPCltSocket* pOutSocket);
	~CTCPSockPair();
	
	//
	// Members.
	//
	uint			m_nInstance;	// The pair instance ID.
	CSockConfig*	m_pConfig;		// The socket config.
	CTCPCltSocket*	m_pInpSocket;	// The client connection.
	CTCPCltSocket*	m_pOutSocket;	// The server connection.
	CPath			m_strSendFile;	// Path of log file for data sent.
	CPath			m_strRecvFile;	// Path of log file for data recieved.
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

inline CTCPSockPair::CTCPSockPair(uint nInstance, CSockConfig* pConfig, CTCPCltSocket* pInpSocket, CTCPCltSocket* pOutSocket)
	: m_nInstance(nInstance)
	, m_pConfig(pConfig)
	, m_pInpSocket(pInpSocket)
	, m_pOutSocket(pOutSocket)
{
	ASSERT(pConfig != NULL);

	// Apply instance number to log file names.
	m_strSendFile.Format(m_pConfig->m_strSendFile, m_nInstance);
	m_strRecvFile.Format(m_pConfig->m_strRecvFile, m_nInstance);

	CPath strAppDir = CPath::ApplicationDir();

	// Prepend app folder.
	m_strSendFile = strAppDir + m_strSendFile;
	m_strRecvFile = strAppDir + m_strRecvFile;
}

inline CTCPSockPair::~CTCPSockPair()
{
	delete m_pInpSocket;
	delete m_pOutSocket;
}

#endif // TCPSOCKPAIR_HPP
