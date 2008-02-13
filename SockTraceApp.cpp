/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKTRACEAPP.CPP
** COMPONENT:	The Application.
** DESCRIPTION:	The CSockTraceApp class definition.
**
*******************************************************************************
*/

#include "Common.hpp"
#include "SockTraceApp.hpp"
#include <Legacy/STLUtils.hpp>
#include <WCL/File.hpp>
#include <WCL/FileException.hpp>
#include <NCL/WinSock.hpp>
#include <WCL/BusyCursor.hpp>
#include "SockConfig.hpp"
#include <NCL/Socket.hpp>
#include <NCL/SocketException.hpp>
#include <NCL/TCPSvrSocket.hpp>
#include <NCL/TCPCltSocket.hpp>
#include "TCPSockPair.hpp"
#include "UDPSockPair.hpp"
#include <WCL/DateTime.hpp>
#include <Core/AnsiWide.hpp>

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
const tchar* CSockTraceApp::VERSION = TXT("v1.6 Beta [Debug]");
#else
const tchar* CSockTraceApp::VERSION = TXT("v1.6 Beta");
#endif

const tchar* CSockTraceApp::INI_FILE_VER = TXT("1.0");

const bool  CSockTraceApp::DEF_TRAY_ICON       = false;
const bool  CSockTraceApp::DEF_MIN_TO_TRAY     = false;
const bool  CSockTraceApp::DEF_TRACE_CONNS     = true;
const bool  CSockTraceApp::DEF_TRACE_DATA      = false;
const bool  CSockTraceApp::DEF_TRACE_TO_WINDOW = true;
const int   CSockTraceApp::DEF_TRACE_LINES     = 100;
const bool  CSockTraceApp::DEF_TRACE_TO_FILE   = false;
const tchar* CSockTraceApp::DEF_TRACE_FILE     = TXT("SockTrace.log");

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
	, m_bTrayIcon(DEF_TRAY_ICON)
	, m_bMinToTray(DEF_MIN_TO_TRAY)
	, m_bTraceConns(DEF_TRACE_CONNS)
	, m_bTraceData(DEF_TRACE_DATA)
	, m_bTraceToWindow(DEF_TRACE_TO_WINDOW)
	, m_nTraceLines(DEF_TRACE_LINES)
	, m_bTraceToFile(DEF_TRACE_TO_FILE)
	, m_strTraceFile(DEF_TRACE_FILE)
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
	DeleteAll(m_aoConfigs);
	DeleteAll(m_aoTCPSvrSocks);
	DeleteAll(m_aoTCPCltSocks);
	DeleteAll(m_aoUDPSvrSocks);
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
	HWND hPrevWnd = NULL;

	// Only allow a single instance.
	if ((hPrevWnd = ::FindWindow(CAppWnd::WNDCLASS_NAME, NULL)) != NULL)
	{
		// If not visible OR minimised, restore it.
		if (!::IsWindowVisible(hPrevWnd) || ::IsIconic(hPrevWnd))
		{
			::ShowWindow(hPrevWnd, SW_RESTORE);
			::SetForegroundWindow(hPrevWnd);
		}

		return false;
	}

	// Set the app title.
	m_strTitle = TXT("Socket Trace");

	// Load settings.
	LoadConfig();

	try
	{
		// Create full path to trace file.
		m_strTracePath = CPath(CPath::ApplicationDir(), m_strTraceFile);

		if (m_bTraceToFile)
		{
			CFile fFile;

			// Truncate the trace file.
			fFile.Create(m_strTracePath);
			fFile.Close();
		}
	}
	catch (CFileException& e)
	{
		AlertMsg(TXT("Failed to truncate the log file:\n\n%s"), e.ErrorText());
		return false;
	}

	// Create the main window.
	if (!m_AppWnd.Create())
		return false;

	// Show it.
	if (ShowNormal() && !m_rcLastPos.Empty())
		m_AppWnd.Move(m_rcLastPos);

	m_AppWnd.Show(m_iCmdShow);

	// Update UI.
	m_AppCmds.UpdateUI();

	// Initialise WinSock.
	int nResult = CWinSock::Startup(1, 1);

	if (nResult != 0)
	{
		FatalMsg(TXT("Failed to initialise WinSock layer: %d."), nResult);
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
	for (uint i = 0; i < m_aoConfigs.size(); ++i)
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

				Trace(TXT("Resolved hostname %s to %s"), pConfig->m_strDstHost, pConfig->m_strDstAddr);
			}
		}
		catch (CSocketException& e)
		{
			Trace(TXT("Failed to resolve hostname %s - %s"), pConfig->m_strDstHost, CWinSock::ErrorToSymbol(e.m_nWSACode));
		}
	}

	// Create all server sockets.
	for (uint i = 0; i < m_aoConfigs.size(); ++i)
	{
		CSockConfig* pConfig = m_aoConfigs[i];

		// TCP socket?
		if (pConfig->m_nType == SOCK_STREAM)
		{
			// Create the TCP listening socket.
			CTCPSvrSocket* pTCPSvrSock = new CTCPSvrSocket(CSocket::ASYNC);

			try
			{
				Trace(TXT("Opening TCP local socket on port %d"), pConfig->m_nSrcPort);

				// Start listening...
				pTCPSvrSock->Listen(pConfig->m_nSrcPort, 5);

				// Attach event handler.
				pTCPSvrSock->AddServerListener(this);

				// Add to the collection.
				m_aoTCPSvrSocks.push_back(pTCPSvrSock);
			}
			catch (CSocketException& e)
			{
				Trace(TXT("Failed to create local socket on port %d - %s"), pConfig->m_nSrcPort, e.ErrorText());

				delete pTCPSvrSock;
			}
		}
		// UDP socket?
		else if (pConfig->m_nType == SOCK_DGRAM)
		{
			Trace(TXT("Opened UDP sockets on port %d connection %d "), pConfig->m_nSrcPort, m_nInstance);

			// Create the listening sockets.
			CUDPSockPair* pUDPSocks = new CUDPSockPair(pConfig, m_nInstance++);

			// Parse destination IP address.
			pUDPSocks->m_oDstAddr.s_addr = inet_addr(T2A(pConfig->m_strDstAddr));
			pUDPSocks->m_nDstPort        = pConfig->m_nDstPort;

			// Start listening...
			pUDPSocks->m_oInpSocket.Listen(pConfig->m_nSrcPort);
			pUDPSocks->m_oOutSocket.Listen(pConfig->m_nSrcPort+1);

			// Add to the collection.
			m_aoUDPSvrSocks.push_back(pUDPSocks);
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
	DeleteAll(m_aoTCPSvrSocks);
	DeleteAll(m_aoTCPCltSocks);
	DeleteAll(m_aoUDPSvrSocks);
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

void CSockTraceApp::Trace(const tchar* pszMsg, ...)
{
	// Ignore, if no output for trace.
	if ((!m_bTraceToWindow) && (!m_bTraceToFile))
		return;

	CString strMsg;

	// Setup arguments.
	va_list	args;
	va_start(args, pszMsg);
	
	// Format message.
	strMsg.FormatEx(pszMsg, args);

	// Prepend date and time.
	strMsg = CDateTime::Current().ToString() + TXT(" ") + strMsg;

	if (m_bTraceToWindow)
	{
		// Send to trace window.	
		m_AppWnd.m_AppDlg.Trace(strMsg);
	}

	try
	{
		if (m_bTraceToFile)
		{
			CFile fLogFile;

			// Open trace file for appending.
			if (m_strTracePath.Exists())
				fLogFile.Open(m_strTracePath, GENERIC_WRITE);
			else
				fLogFile.Create(m_strTracePath);

			// Write trace message to end of file.
			fLogFile.Seek(0, FILE_END);
			fLogFile.WriteLine(strMsg, ANSI_TEXT);

			fLogFile.Close();
		}
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
		AlertMsg(TXT("Failed to write to log file:\n\n%s"), e.ErrorText());
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
	CString strVer = m_oIniFile.ReadString(TXT("Version"), TXT("Version"), INI_FILE_VER);

	// Read the socket configurations.
	int nConfigs = m_oIniFile.ReadInt(TXT("Sockets"), TXT("Count"), 0);

	for (int i = 0; i < nConfigs; ++i)
	{
		CString strSection, strEntry;

		strEntry.Format(TXT("Socket[%d]"), i);

		// Get the socket config section.
		CString strPort = m_oIniFile.ReadString(TXT("Sockets"), strEntry, TXT(""));

		if (strPort.Empty())
			continue;

		// Format section name.
		strSection.Format(TXT("Port %s"), strPort);

		CSockConfig* pConfig = new CSockConfig;

		// Read the socket settings.
		pConfig->m_strType    = m_oIniFile.ReadString(strSection, TXT("Type"),    TXT("TCP")      );
		pConfig->m_nSrcPort   = m_oIniFile.ReadInt   (strSection, TXT("SrcPort"), 80         );
		pConfig->m_strDstHost = m_oIniFile.ReadString(strSection, TXT("DstHost"), TXT("localhost"));
		pConfig->m_nDstPort   = m_oIniFile.ReadInt   (strSection, TXT("DstPort"), 80         );

		// Get the WinSock socket type.
		pConfig->m_nType = (pConfig->m_strType == TXT("TCP")) ? SOCK_STREAM : SOCK_DGRAM;

		CString strDefSendFile, strDefRecvFile;

		// Format the default log filenames.
		strDefSendFile.Format(TXT("Send_%d_%%d"), pConfig->m_nSrcPort);
		strDefRecvFile.Format(TXT("Recv_%d_%%d"), pConfig->m_nSrcPort);

		// Read the log filenames.
		pConfig->m_strSendFile = m_oIniFile.ReadString(strSection, TXT("Send"), strDefSendFile);
		pConfig->m_strRecvFile = m_oIniFile.ReadString(strSection, TXT("Recv"), strDefRecvFile);

		m_aoConfigs.push_back(pConfig);
	}

	// Read the trace settings.
	m_bTraceConns    = m_oIniFile.ReadBool  (TXT("Trace"), TXT("Connections"), m_bTraceConns   );
	m_bTraceData     = m_oIniFile.ReadBool  (TXT("Trace"), TXT("DataFlow"),    m_bTraceData    );
	m_bTraceToWindow = m_oIniFile.ReadBool  (TXT("Trace"), TXT("ToWindow"),    m_bTraceToWindow);
	m_nTraceLines    = m_oIniFile.ReadInt   (TXT("Trace"), TXT("Lines"),       m_nTraceLines   );
	m_bTraceToFile   = m_oIniFile.ReadBool  (TXT("Trace"), TXT("ToFile"),      m_bTraceToFile  );
	m_strTraceFile   = m_oIniFile.ReadString(TXT("Trace"), TXT("FileName"),    m_strTraceFile  );

	// Read the UI settings.
	m_bTrayIcon  = m_oIniFile.ReadBool(TXT("UI"), TXT("TrayIcon"),  m_bTrayIcon );
	m_bMinToTray = m_oIniFile.ReadBool(TXT("UI"), TXT("MinToTray"), m_bMinToTray);

	m_rcLastPos.left   = m_oIniFile.ReadInt(TXT("UI"), TXT("Left"),   0);
	m_rcLastPos.top    = m_oIniFile.ReadInt(TXT("UI"), TXT("Top"),    0);
	m_rcLastPos.right  = m_oIniFile.ReadInt(TXT("UI"), TXT("Right"),  0);
	m_rcLastPos.bottom = m_oIniFile.ReadInt(TXT("UI"), TXT("Bottom"), 0);
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
	m_oIniFile.WriteString(TXT("Version"), TXT("Version"), INI_FILE_VER);

	// Write the socket settings.
	if (m_bCfgModified)
	{
		m_oIniFile.WriteInt(TXT("Sockets"), TXT("Count"), m_aoConfigs.size());

		for (uint i = 0; i < m_aoConfigs.size(); ++i)
		{
			CString strSection, strEntry, strValue;

			CSockConfig* pConfig = m_aoConfigs[i];
		
			strEntry.Format(TXT("Socket[%d]"), i);
			strValue.Format(TXT("%d"), pConfig->m_nSrcPort);

			// Write the socket config section.
			m_oIniFile.WriteString(TXT("Sockets"), strEntry, strValue);

			// Format section name.
			strSection.Format(TXT("Port %s"), strValue);

			// Write the socket settings.
			m_oIniFile.WriteString(strSection, TXT("Type"),    pConfig->m_strType   );
			m_oIniFile.WriteInt   (strSection, TXT("SrcPort"), pConfig->m_nSrcPort  );
			m_oIniFile.WriteString(strSection, TXT("DstHost"), pConfig->m_strDstHost);
			m_oIniFile.WriteInt   (strSection, TXT("DstPort"), pConfig->m_nDstPort  );

			// Read the log filenames.
			m_oIniFile.WriteString(strSection, TXT("Send"), pConfig->m_strSendFile);
			m_oIniFile.WriteString(strSection, TXT("Recv"), pConfig->m_strRecvFile);
		}

		// Write the trace settings.
		m_oIniFile.WriteBool  (TXT("Trace"), TXT("Connections"), m_bTraceConns   );
		m_oIniFile.WriteBool  (TXT("Trace"), TXT("DataFlow"),    m_bTraceData    );
		m_oIniFile.WriteBool  (TXT("Trace"), TXT("ToWindow"),    m_bTraceToWindow);
		m_oIniFile.WriteInt   (TXT("Trace"), TXT("Lines"),       m_nTraceLines   );
		m_oIniFile.WriteBool  (TXT("Trace"), TXT("ToFile"),      m_bTraceToFile  );
		m_oIniFile.WriteString(TXT("Trace"), TXT("FileName"),    m_strTraceFile  );

		// Write the UI settings.
		m_oIniFile.WriteBool(TXT("UI"), TXT("TrayIcon"),  m_bTrayIcon );
		m_oIniFile.WriteBool(TXT("UI"), TXT("MinToTray"), m_bMinToTray);
	}

	// Write the window pos and size.
	m_oIniFile.WriteInt(TXT("UI"), TXT("Left"),   m_rcLastPos.left  );
	m_oIniFile.WriteInt(TXT("UI"), TXT("Top"),    m_rcLastPos.top   );
	m_oIniFile.WriteInt(TXT("UI"), TXT("Right"),  m_rcLastPos.right );
	m_oIniFile.WriteInt(TXT("UI"), TXT("Bottom"), m_rcLastPos.bottom);
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
	for (uint i = 0; i < m_aoConfigs.size(); ++i)
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

			if (App.m_bTraceConns)
				Trace(TXT("Connection accepted from %s"), pInpSocket->Host());

			// Find the config for the server socket.
			CSockConfig* pConfig = FindConfig(pSvrSocket->Type(), pSvrSocket->Port());

			ASSERT(pConfig != NULL);

			// Create socket to destination.
			pOutSocket = new CTCPCltSocket(CSocket::ASYNC);

			pOutSocket->Connect(pConfig->m_strDstHost, pConfig->m_nDstPort);

			if (App.m_bTraceConns)
				Trace(TXT("Opened TCP connection %d to server %s:%d"), m_nInstance, pOutSocket->Host(), pOutSocket->Port());

			// Attach event handler.
			pInpSocket->AddClientListener(this);
			pOutSocket->AddClientListener(this);

			CTCPSockPair* pSockPair = new CTCPSockPair(pConfig, m_nInstance++, pInpSocket, pOutSocket);
			
			// Add socket pair to collection.
			m_aoTCPCltSocks.push_back(pSockPair);

			// Add socket pair to map.
			m_oSockMap.insert(std::make_pair(pInpSocket, pSockPair));
			m_oSockMap.insert(std::make_pair(pOutSocket, pSockPair));
		}
	}
	catch (CSocketException& e)
	{
		Trace(TXT("Failed to accept client connection on port %d  - %s"), pSvrSocket->Port(), e.ErrorText());

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
	// Find the client <-> server pair.
	CSocketMap::const_iterator it = m_oSockMap.find(pSocket);

	ASSERT(it != m_oSockMap.end());

	CSockPair* pPair = it->second;

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
					if (App.m_bTraceData)
						Trace(TXT("Forwarded %d bytes to the server on connection %d"), nRead, pSockPair->m_nInstance);

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
					if (App.m_bTraceData)
						Trace(TXT("Returned %d bytes to the client on connection %d"), nRead, pSockPair->m_nInstance);

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
					if (App.m_bTraceData)
						Trace(TXT("Forwarded %d bytes to the server (%s:%d)"), nRead, inet_ntoa(pSockPair->m_oSrcAddr), pSockPair->m_nSrcPort);

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
					if (App.m_bTraceData)
						Trace(TXT("Returned %d bytes to the client (%s:%d)"), nRead, inet_ntoa(oAddress), nPort);

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
		Trace(TXT("Failed to forward packets on connection %d - %s"), pPair->m_nInstance, e.ErrorText());
	}
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
	// Find the client <-> server pair.
	CSocketMap::const_iterator it = m_oSockMap.find(pSocket);

	ASSERT(it != m_oSockMap.end());

	CSockPair* pSockPair = it->second;

	ASSERT(pSockPair != NULL);
	ASSERT(pSockPair->m_pConfig->m_nType == SOCK_STREAM);

	// Must be a TCP pair.
	CTCPSockPair* pTCPSockPair = static_cast<CTCPSockPair*>(pSockPair);

	if (App.m_bTraceConns)
		Trace(TXT("Connection %d closed on port %d"), pSockPair->m_nInstance, pSockPair->m_pConfig->m_nSrcPort);

	// Detach event handler.
	pTCPSockPair->m_pInpSocket->RemoveClientListener(this);
	pTCPSockPair->m_pOutSocket->RemoveClientListener(this);

	// Close both sockets.
	pTCPSockPair->m_pInpSocket->Close();
	pTCPSockPair->m_pOutSocket->Close();

	// Remove socket pair from map.
	m_oSockMap.erase(m_oSockMap.find(pTCPSockPair->m_pInpSocket));
	m_oSockMap.erase(m_oSockMap.find(pTCPSockPair->m_pOutSocket));
}

/******************************************************************************
** Method:		OnError()
**
** Description:	An error has occurred on the socket.
**
** Parameters:	pSocket		The socket.
**				nEvent		The socket event type.
**				nError		The error code.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::OnError(CSocket* pSocket, int nEvent, int nError)
{
	// Find the client <-> server pair.
	CSocketMap::const_iterator it = m_oSockMap.find(pSocket);

	ASSERT(it != m_oSockMap.end());

	CSockPair* pSockPair = it->second;

	ASSERT(pSockPair != NULL);

	Trace(TXT("%s Error on connection %d: %s"), CSocket::AsyncEventStr(nEvent), pSockPair->m_nInstance, CWinSock::ErrorToSymbol(nError));
}
