/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKTRACEAPP.CPP
** COMPONENT:	The Application.
** DESCRIPTION:	The CSockTraceApp class definition.
**
*******************************************************************************
*/

#include "AppHeaders.hpp"

#ifdef _DEBUG
// For memory leak detection.
#define new DBGCRT_NEW
#endif

/******************************************************************************
**
** Global variables.
**
*******************************************************************************
*/

// "The" application object.
CSockTraceApp App;

/******************************************************************************
**
** Class members.
**
*******************************************************************************
*/

#ifdef _DEBUG
const char* CSockTraceApp::VERSION      = "v1.5 [Debug]";
#else
const char* CSockTraceApp::VERSION      = "v1.5";
#endif

const char* CSockTraceApp::INI_FILE_VER  = "1.0";

/******************************************************************************
** Method:		Constructor
**
** Description:	Default constructor.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

CSockTraceApp::CSockTraceApp()
	: CApp(m_AppWnd, m_AppCmds)
	, m_nInstance(1)
	, m_bCfgModified(false)
{
}

/******************************************************************************
** Method:		Destructor
**
** Description:	Cleanup.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

CSockTraceApp::~CSockTraceApp()
{
	// Cleanup arrays.
	m_aoConfigs.DeleteAll();
	m_aoTCPSvrSocks.DeleteAll();
	m_aoTCPCltSocks.DeleteAll();
	m_aoUDPSvrSocks.DeleteAll();
}

/******************************************************************************
** Method:		OnOpen()
**
** Description:	Initialises the application.
**
** Parameters:	None.
**
** Returns:		true or false.
**
*******************************************************************************
*/

bool CSockTraceApp::OnOpen()
{
	// Set the app title.
	m_strTitle = "Socket Trace";

	// Load settings.
	LoadConfig();

	try
	{
		// Create the application trace file.
		m_strTraceFile = CPath(CPath::ApplicationDir(), "SockTrace.log");

		CFile fFile;

		fFile.Create(m_strTraceFile);
		fFile.Close();
	}
	catch (CFileException& e)
	{
		AlertMsg("Failed to truncate the log file:\n\n%s", e.ErrorText());
		return false;
	}

	// Create the main window.
	if (!m_AppWnd.Create())
		return false;

	// Show it.
	if ( (m_iCmdShow == SW_SHOWNORMAL) && (m_rcLastPos.Empty() == false) )
		m_AppWnd.Move(m_rcLastPos);

	m_AppWnd.Show(m_iCmdShow);

	// Update UI.
	m_AppCmds.UpdateUI();

	// Initialise WinSock.
	int nResult = CWinSock::Startup(1, 1);

	if (nResult != 0)
	{
		FatalMsg("Failed to initialise WinSock layer: %d.", nResult);
		return false;
	}

	// Start listening...
	OpenSockets();

	return true;
}

/******************************************************************************
** Method:		OnClose()
**
** Description:	Cleans up the application resources.
**
** Parameters:	None.
**
** Returns:		true or false.
**
*******************************************************************************
*/

bool CSockTraceApp::OnClose()
{
	// Stop listening...
	CloseSockets();

	// Terminate WinSock.
	CWinSock::Cleanup();

	// Save settings.
	SaveConfig();

	return true;
}

/******************************************************************************
** Method:		OpenSockets()
**
** Description:	Opens all server side sockets.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::OpenSockets()
{
	CBusyCursor oCursor;

	// Resolve all destination hostnames...
	for (int i = 0; i < m_aoConfigs.Size(); ++i)
	{
		CSockConfig* pConfig = m_aoConfigs[i];

		try
		{
			// Assume its an IP address.
			pConfig->m_strDstAddr = pConfig->m_strDstHost;

			// Needs resolving?
			if (!CSocket::IsAddress(pConfig->m_strDstHost))
			{
				pConfig->m_strDstAddr = CSocket::ResolveStr(pConfig->m_strDstHost);

				Trace("Resolved hostname %s to %s", pConfig->m_strDstHost, pConfig->m_strDstAddr);
			}
		}
		catch (CSocketException& e)
		{
			Trace("Failed to resolve hostname %s - %s", pConfig->m_strDstHost, CWinSock::ErrorToSymbol(e.m_nWSACode));
		}
	}

	// Create all server sockets.
	for (i = 0; i < m_aoConfigs.Size(); ++i)
	{
		CSockConfig* pConfig = m_aoConfigs[i];

		// TCP socket?
		if (pConfig->m_nType == SOCK_STREAM)
		{
			// Create the TCP listening socket.
			CTCPSvrSocket* pTCPSvrSock = new CTCPSvrSocket(CSocket::ASYNC);

			try
			{
				Trace("Opening TCP local socket on port %d", pConfig->m_nSrcPort);

				// Start listening...
				pTCPSvrSock->Listen(pConfig->m_nSrcPort, 5);

				// Attach event handler.
				pTCPSvrSock->AddServerListener(this);

				// Add to the collection.
				m_aoTCPSvrSocks.Add(pTCPSvrSock);
			}
			catch (CSocketException& e)
			{
				Trace("Failed to create local socket on port %d - %s", pConfig->m_nSrcPort, e.ErrorText());

				delete pTCPSvrSock;
			}
		}
		// UDP socket?
		else if (pConfig->m_nType == SOCK_DGRAM)
		{
			Trace("Opened UDP sockets on port %d connection %d ", pConfig->m_nSrcPort, m_nInstance);

			// Create the listening sockets.
			CUDPSockPair* pUDPSocks = new CUDPSockPair(pConfig, m_nInstance++);

			// Parse destination IP address.
			pUDPSocks->m_oDstAddr.s_addr = inet_addr(pConfig->m_strDstAddr);
			pUDPSocks->m_nDstPort        = pConfig->m_nDstPort;

			// Start listening...
			pUDPSocks->m_oInpSocket.Listen(pConfig->m_nSrcPort);
			pUDPSocks->m_oOutSocket.Listen(pConfig->m_nSrcPort+1);

			// Add to the collection.
			m_aoUDPSvrSocks.Add(pUDPSocks);
		}
	}
}

/******************************************************************************
** Method:		CloseSockets()
**
** Description:	Close all client and server side sockets.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::CloseSockets()
{
	// Cleanup the sockets.
	m_aoTCPSvrSocks.DeleteAll();
	m_aoTCPCltSocks.DeleteAll();
	m_aoUDPSvrSocks.DeleteAll();
}

/******************************************************************************
** Method:		Trace()
**
** Description:	Displays a trace message in the trace window.
**
** Parameters:	pszMsg			The message format.
**				...				Variable number of arguments.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::Trace(const char* pszMsg, ...)
{
	CString strMsg;

	// Setup arguments.
	va_list	args;
	va_start(args, pszMsg);
	
	// Format message.
	strMsg.FormatEx(pszMsg, args);

	// Prepend date and time.
	strMsg = CDateTime::Current().ToString() + " " + strMsg;

	// Send to trace window.	
	m_AppWnd.m_AppDlg.Trace(strMsg);

	try
	{
		CFile fLogFile;

		// Open application log file.
		if (m_strTraceFile.Exists())
			fLogFile.Open(m_strTraceFile, GENERIC_WRITE);
		else
			fLogFile.Create(m_strTraceFile);

		// Write trace message to application log file.
		fLogFile.Seek(0, FILE_END);
		fLogFile.WriteLine(strMsg);
		fLogFile.Close();
	}
	catch (CFileException& /*e*/)
	{ }
}

/******************************************************************************
** Method:		LogData()
**
** Description:	Write socket data to the log file.
**
** Parameters:	strFileName		The log file name.
**				pvData			The data buffer.
**				nLength			The data length.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::LogData(CPath& strFileName, const void* pvData, uint nLength)
{
	try
	{
		CFile fLogFile;

		if (strFileName.Exists())
			fLogFile.Open(strFileName, GENERIC_WRITE);
		else
			fLogFile.Create(strFileName);

		fLogFile.Seek(0, FILE_END);
		fLogFile.Write(pvData, nLength);
		fLogFile.Close();
	}
	catch (CFileException& e)
	{
		AlertMsg("Failed to write to log file:\n\n%s", e.ErrorText());
	}
}

/******************************************************************************
** Method:		LoadConfig()
**
** Description:	Load the app configuration.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::LoadConfig()
{
	// Read the file version.
	CString strVer = m_oIniFile.ReadString("Version", "Version", INI_FILE_VER);

	// Read the socket configurations.
	int nConfigs = m_oIniFile.ReadInt("Sockets", "Count", 0);

	for (int i = 0; i < nConfigs; ++i)
	{
		CString strSection, strEntry;

		strEntry.Format("Socket[%d]", i);

		// Get the socket config section.
		CString strPort = m_oIniFile.ReadString("Sockets", strEntry, "");

		if (strPort.Empty())
			continue;

		// Format section name.
		strSection.Format("Port %s", strPort);

		CSockConfig* pConfig = new CSockConfig;

		// Read the socket settings.
		pConfig->m_strType    = m_oIniFile.ReadString(strSection, "Type",    "TCP"      );
		pConfig->m_nSrcPort   = m_oIniFile.ReadInt   (strSection, "SrcPort", 80         );
		pConfig->m_strDstHost = m_oIniFile.ReadString(strSection, "DstHost", "localhost");
		pConfig->m_nDstPort   = m_oIniFile.ReadInt   (strSection, "DstPort", 80         );

		// Get the WinSock socket type.
		pConfig->m_nType = (pConfig->m_strType == "TCP") ? SOCK_STREAM : SOCK_DGRAM;

		CString strDefSendFile, strDefRecvFile;

		// Format the default log filenames.
		strDefSendFile.Format("Send_%d_%%d", pConfig->m_nSrcPort);
		strDefRecvFile.Format("Recv_%d_%%d", pConfig->m_nSrcPort);

		// Read the log filenames.
		pConfig->m_strSendFile = m_oIniFile.ReadString(strSection, "Send", strDefSendFile);
		pConfig->m_strRecvFile = m_oIniFile.ReadString(strSection, "Recv", strDefRecvFile);

		m_aoConfigs.Add(pConfig);
	}

	// Read the window pos and size.
	m_rcLastPos.left   = m_oIniFile.ReadInt("UI", "Left",   0);
	m_rcLastPos.top    = m_oIniFile.ReadInt("UI", "Top",    0);
	m_rcLastPos.right  = m_oIniFile.ReadInt("UI", "Right",  0);
	m_rcLastPos.bottom = m_oIniFile.ReadInt("UI", "Bottom", 0);
}

/******************************************************************************
** Method:		SaveConfig()
**
** Description:	Save the app configuration.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::SaveConfig()
{
	// Write the file version.
	m_oIniFile.WriteString("Version", "Version", INI_FILE_VER);

	// Write the socket settings.
	if (m_bCfgModified)
	{
		m_oIniFile.WriteInt("Sockets", "Count", m_aoConfigs.Size());

		for (int i = 0; i < m_aoConfigs.Size(); ++i)
		{
			CString strSection, strEntry, strValue;

			CSockConfig* pConfig = m_aoConfigs[i];
		
			strEntry.Format("Socket[%d]", i);
			strValue.Format("%d", pConfig->m_nSrcPort);

			// Write the socket config section.
			m_oIniFile.WriteString("Sockets", strEntry, strValue);

			// Format section name.
			strSection.Format("Port %s", strValue);

			// Write the socket settings.
			m_oIniFile.WriteString(strSection, "Type",    pConfig->m_strType   );
			m_oIniFile.WriteInt   (strSection, "SrcPort", pConfig->m_nSrcPort  );
			m_oIniFile.WriteString(strSection, "DstHost", pConfig->m_strDstHost);
			m_oIniFile.WriteInt   (strSection, "DstPort", pConfig->m_nDstPort  );

			// Read the log filenames.
			m_oIniFile.WriteString(strSection, "Send", pConfig->m_strSendFile);
			m_oIniFile.WriteString(strSection, "Recv", pConfig->m_strRecvFile);
		}
	}

	// Write the window pos and size.
	m_oIniFile.WriteInt("UI", "Left",   m_rcLastPos.left  );
	m_oIniFile.WriteInt("UI", "Top",    m_rcLastPos.top   );
	m_oIniFile.WriteInt("UI", "Right",  m_rcLastPos.right );
	m_oIniFile.WriteInt("UI", "Bottom", m_rcLastPos.bottom);
}

/******************************************************************************
** Method:		FindConfig()
**
** Description:	Find a socket config.
**
** Parameters:	nType	The socket type.
**				nPort	The port number.
**
** Returns:		The config or NULL.
**
*******************************************************************************
*/

CSockConfig* CSockTraceApp::FindConfig(int nType, uint nPort) const
{
	// For all configs...
	for (int i = 0; i < m_aoConfigs.Size(); ++i)
	{
		CSockConfig* pConfig = m_aoConfigs[i];

		if ( (pConfig->m_nType == nType) && (pConfig->m_nSrcPort == nPort) )
			return pConfig;
	}

	return NULL;
}

/******************************************************************************
** Method:		OnAcceptReady()
**
** Description:	The TCP server socket has a connection waiting.
**
** Parameters:	pSocket		The TCP server socket.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::OnAcceptReady(CTCPSvrSocket* pSvrSocket)
{
	CTCPCltSocket* pInpSocket = NULL;
	CTCPCltSocket* pOutSocket = NULL;

	try
	{
		// Connection still waiting?
		if (pSvrSocket->CanAccept())
		{
			// Accept the connection.
			pInpSocket = pSvrSocket->Accept();

			ASSERT(pInpSocket != NULL);

			Trace("Connection accepted from %s", pInpSocket->Host());

			// Find the config for the server socket.
			CSockConfig* pConfig = FindConfig(pSvrSocket->Type(), pSvrSocket->Port());

			ASSERT(pConfig != NULL);

			// Create socket to destination.
			pOutSocket = new CTCPCltSocket(CSocket::ASYNC);

			pOutSocket->Connect(pConfig->m_strDstHost, pConfig->m_nDstPort);

			Trace("Opened TCP connection %d to server %s:%d", m_nInstance, pOutSocket->Host(), pOutSocket->Port());

			// Attach event handler.
			pInpSocket->AddClientListener(this);
			pOutSocket->AddClientListener(this);

			CTCPSockPair* pSockPair = new CTCPSockPair(pConfig, m_nInstance++, pInpSocket, pOutSocket);
			
			// Add socket pair to collection.
			m_aoTCPCltSocks.Add(pSockPair);

			// Add socket pair to map.
			m_oSockMap.Add(pInpSocket, pSockPair);
			m_oSockMap.Add(pOutSocket, pSockPair);
		}
	}
	catch (CSocketException& e)
	{
		Trace("Failed to accept client connection on port %d  - %s", pSvrSocket->Port(), e.ErrorText());

		// Cleanup.
		delete pInpSocket;
		delete pOutSocket;
	}
}

/******************************************************************************
** Method:		OnReadReady()
**
** Description:	The socket has data to read.
**
** Parameters:	pSocket		The socket.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::OnReadReady(CSocket* pSocket)
{
	CSockPair* pPair = NULL;

	// Find the client <-> server pair.
	m_oSockMap.Find(pSocket, pPair);

	ASSERT(pPair != NULL);

	try
	{
		// TCP socket pair?
		if (pPair->m_pConfig->m_nType == SOCK_STREAM)
		{
			CTCPSockPair* pSockPair = static_cast<CTCPSockPair*>(pPair);

			int nAvail, nRead;

			// Check client side socket.
			if ( (pSocket == pSockPair->m_pInpSocket)
			  && ((nAvail = pSockPair->m_pInpSocket->Available()) > 0) )
			{
				CBuffer oBuffer(nAvail);

				// Read data from the client socket.
				if ((nRead = pSockPair->m_pInpSocket->Recv(oBuffer)) > 0)
				{
					Trace("Forwarded %d bytes to the server on connection %d", nRead, pSockPair->m_nInstance);

					// Send down the server socket.
					pSockPair->m_pOutSocket->Send(oBuffer.Buffer(), nRead);

					// Log the data sent by the client.
					LogData(pSockPair->m_strSendFile, oBuffer.Buffer(), nRead);

					// Update stats.
					pSockPair->m_nBytesSent += nRead;
				}
			}

			// Check server side socket.
			if ( (pSocket == pSockPair->m_pOutSocket)
			  && ((nAvail = pSockPair->m_pOutSocket->Available()) > 0) )
			{
				CBuffer oBuffer(nAvail);

				// Read data from the server socket.
				if ((nRead = pSockPair->m_pOutSocket->Recv(oBuffer)) > 0)
				{
					Trace("Forwarded %d bytes to the client on connection %d", nRead, pSockPair->m_nInstance);

					// Send down the client socket.
					pSockPair->m_pInpSocket->Send(oBuffer.Buffer(), nRead);

					// Log the data received by the client.
					LogData(pSockPair->m_strRecvFile, oBuffer.Buffer(), nRead);

					// Update stats.
					pSockPair->m_nBytesRecv += nRead;
				}
			}
		}
		// UDP socket pair?
		else if (pPair->m_pConfig->m_nType == SOCK_DGRAM)
		{
			CUDPSockPair* pSockPair = static_cast<CUDPSockPair*>(pPair);

			int nAvail, nRead;

			// Check client side socket.
			if ( (pSocket == &pSockPair->m_oInpSocket)
			  && ((nAvail = pSockPair->m_oInpSocket.Available()) > 0) )
			{
				CBuffer oBuffer(nAvail);

				// Read data from the client socket.
				if ((nRead = pSockPair->m_oInpSocket.RecvFrom(oBuffer, pSockPair->m_oSrcAddr, pSockPair->m_nSrcPort)) > 0)
				{
					Trace("Forwarded %d bytes to the server (%s:%d)", nRead, inet_ntoa(pSockPair->m_oSrcAddr), pSockPair->m_nSrcPort);

					// Send down the server socket.
					pSockPair->m_oOutSocket.SendTo(oBuffer.Buffer(), nRead, pSockPair->m_oDstAddr, pSockPair->m_nDstPort);

					// Log the data sent by the client.
					LogData(pSockPair->m_strSendFile, oBuffer.Buffer(), nRead);

					// Update stats.
					pSockPair->m_nBytesSent += nRead;
				}
			}

			// Check server side socket.
			if ( (pSocket == &pSockPair->m_oOutSocket)
			  && ((nAvail = pSockPair->m_oOutSocket.Available()) > 0) )
			{
				CBuffer oBuffer(nAvail);
				in_addr oAddress;
				uint    nPort;

				// Read data from the server socket.
				if ((nRead = pSockPair->m_oOutSocket.RecvFrom(oBuffer, oAddress, nPort)) > 0)
				{
					Trace("Forwarded %d bytes to the client (%s:%d)", nRead, inet_ntoa(oAddress), nPort);

					// Send down the client socket.
					pSockPair->m_oInpSocket.SendTo(oBuffer.Buffer(), nRead, pSockPair->m_oSrcAddr, pSockPair->m_nSrcPort);

					// Log the data received by the client.
					LogData(pSockPair->m_strRecvFile, oBuffer.Buffer(), nRead);

					// Update stats.
					pSockPair->m_nBytesRecv += nRead;
				}
			}
		}
	}
	catch (CSocketException& e)
	{
		Trace("Failed to forward packets on connection %d - %s", pPair->m_nInstance, e.ErrorText());
	}
}

/******************************************************************************
** Method:		OnWriteReady()
**
** Description:	The socket has space to write.
**
** Parameters:	pSocket		The socket.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::OnWriteReady(CSocket* /*pSocket*/)
{
}

/******************************************************************************
** Method:		OnClosed()
**
** Description:	The socket has been closed.
**
** Parameters:	pSocket		The socket.
**				nReason		The reason for closure.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::OnClosed(CSocket* pSocket, int /*nReason*/)
{
	CSockPair* pSockPair = NULL;

	// Find the client <-> server pair.
	m_oSockMap.Find(pSocket, pSockPair);

	ASSERT(pSockPair != NULL);
	ASSERT(pSockPair->m_pConfig->m_nType == SOCK_STREAM);

	// Must be a TCP pair.
	CTCPSockPair* pTCPSockPair = static_cast<CTCPSockPair*>(pSockPair);

	Trace("Connection %d closed on port %d", pSockPair->m_nInstance, pSockPair->m_pConfig->m_nSrcPort);

	// Detach event handler.
	pTCPSockPair->m_pInpSocket->RemoveClientListener(this);
	pTCPSockPair->m_pOutSocket->RemoveClientListener(this);

	// Close both sockets.
	pTCPSockPair->m_pInpSocket->Close();
	pTCPSockPair->m_pOutSocket->Close();

	// Remove socket pair from map.
	m_oSockMap.Remove(pTCPSockPair->m_pInpSocket);
	m_oSockMap.Remove(pTCPSockPair->m_pOutSocket);
}
