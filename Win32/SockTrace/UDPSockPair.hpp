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

/******************************************************************************
** 
** The pair of sockets used to proxy a UDP client/server connection.
**
*******************************************************************************
*/

class CUDPSockPair
{
public:
	//
	// Constructors/Destructor.
	//
	CUDPSockPair(uint nInstance, CSockConfig* pConfig);
	~CUDPSockPair();
	
	//
	// Members.
	//
	uint			m_nInstance;	// The pair instance ID.
	CSockConfig*	m_pConfig;		// The socket config.
	CUDPSvrSocket	m_oInpSocket;	// The client connection.
	CUDPSvrSocket	m_oOutSocket;	// The server connection.
	CPath			m_strSendFile;	// Path of log file for data sent.
	CPath			m_strRecvFile;	// Path of log file for data recieved.
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

inline CUDPSockPair::CUDPSockPair(uint nInstance, CSockConfig* pConfig)
	: m_nInstance(nInstance)
	, m_pConfig(pConfig)
{
	ASSERT(pConfig != NULL);

	// Apply instance number to log file names.
	m_strSendFile.Format(m_pConfig->m_strSendFile, m_nInstance);
	m_strRecvFile.Format(m_pConfig->m_strRecvFile, m_nInstance);

	CPath strAppDir = CPath::ApplicationDir();

	// Prepend app folder.
	m_strSendFile = strAppDir + m_strSendFile;
	m_strRecvFile = strAppDir + m_strRecvFile;

	// Reset source address and port.
	m_oSrcAddr.s_addr = INADDR_NONE;
	m_nSrcPort        = 0;
}

inline CUDPSockPair::~CUDPSockPair()
{
}

#endif // UDPSOCKPAIR_HPP
