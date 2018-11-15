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
#include <WCL/AppConfig.hpp>
#include <Core/ConfigurationException.hpp>
#include <limits>
#include <shlobj.h>

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

//! The configuration data publisher name.
const tchar* PUBLISHER = TXT("Chris Oldwood");
//! The configuration data application name.
const tchar* APPLICATION = TXT("Socket Trace");
//! The configuration data format version.
const tchar* CONFIG_VERSION = TXT("1.1");

const bool  CSockTraceApp::DEF_TRAY_ICON       = false;
const bool  CSockTraceApp::DEF_MIN_TO_TRAY     = false;
const bool  CSockTraceApp::DEF_TRACE_CONNS     = true;
const bool  CSockTraceApp::DEF_TRACE_DATA      = false;
const bool  CSockTraceApp::DEF_TRACE_TO_WINDOW = true;
const uint  CSockTraceApp::DEF_TRACE_LINES     = 100;
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
	, m_AppWnd()
	, m_AppCmds(m_AppWnd)
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
	CloseSockets();
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
	m_strTitle = APPLICATION;

	// Construct the folder for the data files.
	m_appDataFolder = CPath::SpecialDir(CSIDL_APPDATA) / APPLICATION;

	try
	{
		// Load settings.
		loadConfig();
	}
	catch (const Core::Exception& e)
	{
		FatalMsg(TXT("Failed to configure the application:-\n\n%s"), e.twhat());
		return false;
	}

	try
	{
		// Create full path to trace file.
		m_strTracePath = m_appDataFolder / m_strTraceFile;

		if (m_bTraceToFile)
		{
			CFile fFile;

			// Truncate the trace file.
			fFile.Create(m_strTracePath);
			fFile.Close();
		}
	}
	catch (const CFileException& e)
	{
		AlertMsg(TXT("Failed to truncate the log file:\n\n%s"), e.twhat());
		return false;
	}

	// Create the main window.
	if (!m_AppWnd.Create())
		return false;

	// Show it.
	if (!m_rcLastPos.Empty())
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

	try
	{
		// Save settings.
		saveConfig();
	}
	catch (const Core::Exception& e)
	{
		FatalMsg(TXT("Failed to save the application configuration:-\n\n%s"), e.twhat());
		return false;
	}

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
		CSockConfigPtr pConfig = m_aoConfigs[i];

		try
		{
			// Assume its an IP address.
			pConfig->m_strDstAddr = pConfig->m_strDstHost;

			// Needs resolving?
			if (!CSocket::IsAddress(pConfig->m_strDstHost))
			{
				pConfig->m_strDstAddr = CSocket::ResolveStr(pConfig->m_strDstHost);

				Trace(TXT("Resolved hostname %s to %s"), pConfig->m_strDstHost.c_str(), pConfig->m_strDstAddr.c_str());
			}
		}
		catch (const CSocketException& e)
		{
			Trace(TXT("Failed to resolve hostname %s - %s"), pConfig->m_strDstHost.c_str(), CWinSock::ErrorToSymbol(e.m_nWSACode).c_str());
		}
	}

	// Create all server sockets.
	for (uint i = 0; i < m_aoConfigs.size(); ++i)
	{
		CSockConfigPtr pConfig = m_aoConfigs[i];

		// TCP socket?
		if (pConfig->m_nType == SOCK_STREAM)
		{
			// Create the TCP listening socket.
			CTCPSvrSocketPtr pTCPSvrSock(new CTCPSvrSocket(CSocket::ASYNC));

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
			catch (const CSocketException& e)
			{
				Trace(TXT("Failed to create local socket on port %d - %s"), pConfig->m_nSrcPort, e.twhat());
			}
		}
		// UDP socket?
		else if (pConfig->m_nType == SOCK_DGRAM)
		{
			Trace(TXT("Opened UDP sockets on port %d connection %d "), pConfig->m_nSrcPort, m_nInstance);

			// Create the listening sockets.
			CUDPSockPairPtr pUDPSocks(new CUDPSockPair(pConfig, m_nInstance++));

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
	m_aoTCPSvrSocks.clear();
	m_aoTCPCltSocks.clear();
	m_aoUDPSvrSocks.clear();
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
			fLogFile.Seek(0, WCL::IStreamBase::END);
			fLogFile.WriteLine(strMsg, ANSI_TEXT);

			fLogFile.Close();
		}
	}
	catch (const CFileException& /*e*/)
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

void CSockTraceApp::LogData(CPath& strFileName, const void* pvData, size_t nLength)
{
	try
	{
		CFile fLogFile;

		if (strFileName.Exists())
			fLogFile.Open(strFileName, GENERIC_WRITE);
		else
			fLogFile.Create(strFileName);

		fLogFile.Seek(0, WCL::IStreamBase::END);
		fLogFile.Write(pvData, nLength);
		fLogFile.Close();
	}
	catch (const CFileException& e)
	{
		AlertMsg(TXT("Failed to write to log file:\n\n%s"), e.twhat());
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

void CSockTraceApp::loadConfig()
{
	CBusyCursor    busyCursor;
	WCL::AppConfig appConfig(PUBLISHER, APPLICATION);

	// Read the config data version.
	tstring version = appConfig.readString(appConfig.DEFAULT_SECTION, TXT("Version"), CONFIG_VERSION);

	if (version != CONFIG_VERSION)
		throw Core::ConfigurationException(Core::fmt(TXT("The configuration data is incompatible - '%s'"), version.c_str()));

	// Read the socket configurations.
	const size_t max = std::numeric_limits<size_t>::max();

	for (size_t i = 0; i != max; ++i)
	{
		tstring entry   = Core::fmt(TXT("%u"), i);
		tstring section = appConfig.readString(TXT("Sockets"), entry, TXT(""));

		if (section.empty())
			break;

		CSockConfigPtr pConfig(new CSockConfig);

		// Read the socket settings.
		pConfig->m_strType    = appConfig.readString(section, TXT("Type"), TXT("TCP"));
		pConfig->m_nSrcPort   = appConfig.readValue<uint>(section, TXT("SrcPort"), 80);
		pConfig->m_strDstHost = appConfig.readString(section, TXT("DstHost"), TXT("localhost"));
		pConfig->m_nDstPort   = appConfig.readValue<uint>(section, TXT("DstPort"), 80);

		// Get the WinSock socket type.
		pConfig->m_nType = (pConfig->m_strType == TXT("TCP")) ? SOCK_STREAM : SOCK_DGRAM;

		CString strDefSendFile, strDefRecvFile;

		// Format the default log filenames.
		strDefSendFile.Format(TXT("Send_%d_%%d"), pConfig->m_nSrcPort);
		strDefRecvFile.Format(TXT("Recv_%d_%%d"), pConfig->m_nSrcPort);

		// Read the log filenames.
		pConfig->m_strSendFile = appConfig.readString(section, TXT("Send"), tstring(strDefSendFile));
		pConfig->m_strRecvFile = appConfig.readString(section, TXT("Recv"), tstring(strDefRecvFile));

		m_aoConfigs.push_back(pConfig);
	}

	// Read the trace settings.
	m_bTraceConns    = appConfig.readValue<bool>(TXT("Trace"), TXT("Connections"), m_bTraceConns   );
	m_bTraceData     = appConfig.readValue<bool>(TXT("Trace"), TXT("DataFlow"), m_bTraceData    );
	m_bTraceToWindow = appConfig.readValue<bool>(TXT("Trace"), TXT("ToWindow"), m_bTraceToWindow);
	m_nTraceLines    = appConfig.readValue<uint>(TXT("Trace"), TXT("Lines"), m_nTraceLines   );
	m_bTraceToFile   = appConfig.readValue<bool>(TXT("Trace"), TXT("ToFile"), m_bTraceToFile  );
	m_strTraceFile   = appConfig.readString(TXT("Trace"), TXT("FileName"), tstring(m_strTraceFile));

	// Read the UI settings.
	m_bTrayIcon  = appConfig.readValue<bool>(TXT("UI"), TXT("TrayIcon"),  m_bTrayIcon );
	m_bMinToTray = appConfig.readValue<bool>(TXT("UI"), TXT("MinToTray"), m_bMinToTray);
	m_rcLastPos  = appConfig.readValue<CRect>(TXT("UI"), TXT("MainWindow"), m_rcLastPos);
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

void CSockTraceApp::saveConfig()
{
	CBusyCursor    busyCursor;
	WCL::AppConfig appConfig(PUBLISHER, APPLICATION);

	// Write the config data version.
	appConfig.writeString(appConfig.DEFAULT_SECTION, TXT("Version"), CONFIG_VERSION);

	// Write the socket settings.
	if (m_bCfgModified)
	{
		appConfig.deleteSection(TXT("Sockets"));

		for (uint i = 0; i < m_aoConfigs.size(); ++i)
		{
			CSockConfigPtr pConfig = m_aoConfigs[i];

			const tchar* prefix = (pConfig->m_nType == SOCK_STREAM) ? TXT("TCP") : TXT("UDP");
		
			tstring entry   = Core::fmt(TXT("%u"), i);
			tstring section = Core::fmt(TXT("%s:%u"), prefix, pConfig->m_nSrcPort);

			// Write the socket config section.
			appConfig.writeString(TXT("Sockets"), entry, section);

			// Write the socket settings.
			appConfig.writeString(section, TXT("Type"), tstring(pConfig->m_strType));
			appConfig.writeValue<uint>(section, TXT("SrcPort"), pConfig->m_nSrcPort);
			appConfig.writeString(section, TXT("DstHost"), tstring(pConfig->m_strDstHost));
			appConfig.writeValue<uint>(section, TXT("DstPort"), pConfig->m_nDstPort);

			// Read the log filenames.
			appConfig.writeString(section, TXT("Send"), tstring(pConfig->m_strSendFile));
			appConfig.writeString(section, TXT("Recv"), tstring(pConfig->m_strRecvFile));
		}

		// Write the trace settings.
		appConfig.writeValue<bool>(TXT("Trace"), TXT("Connections"), m_bTraceConns);
		appConfig.writeValue<bool>(TXT("Trace"), TXT("DataFlow"), m_bTraceData);
		appConfig.writeValue<bool>(TXT("Trace"), TXT("ToWindow"), m_bTraceToWindow);
		appConfig.writeValue<uint>(TXT("Trace"), TXT("Lines"), m_nTraceLines);
		appConfig.writeValue<bool>(TXT("Trace"), TXT("ToFile"), m_bTraceToFile);
		appConfig.writeString(TXT("Trace"), TXT("FileName"), tstring(m_strTraceFile));

		// Write the UI settings.
		appConfig.writeValue<bool>(TXT("UI"), TXT("TrayIcon"), m_bTrayIcon);
		appConfig.writeValue<bool>(TXT("UI"), TXT("MinToTray"), m_bMinToTray);
	}

	// Write the window pos and size.
	appConfig.writeValue<CRect>(TXT("UI"), TXT("MainWindow"), m_rcLastPos);
}

/******************************************************************************
** Method:		FindConfig()
**
** Description:	Find a socket config.
**
** Parameters:	nType	The socket type.
**				nPort	The port number.
**
** Returns:		The config or nullptr.
**
*******************************************************************************
*/

CSockConfigPtr CSockTraceApp::FindConfig(int nType, uint nPort) const
{
	// For all configs...
	for (uint i = 0; i < m_aoConfigs.size(); ++i)
	{
		CSockConfigPtr pConfig = m_aoConfigs[i];

		if ( (pConfig->m_nType == nType) && (pConfig->m_nSrcPort == nPort) )
			return pConfig;
	}

	return CSockConfigPtr();
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
	try
	{
		// Connection still waiting?
		if (pSvrSocket->CanAccept())
		{
			// Accept the connection.
			CTCPCltSocketPtr pInpSocket(pSvrSocket->Accept());

			ASSERT(pInpSocket.get() != nullptr);

			if (App.m_bTraceConns)
				Trace(TXT("Connection accepted from %s"), pInpSocket->Host().c_str());

			// Find the config for the server socket.
			CSockConfigPtr pConfig = FindConfig(pSvrSocket->Type(), pSvrSocket->Port());

			ASSERT(pConfig.get() != nullptr);

			// Create socket to destination.
			CTCPCltSocketPtr pOutSocket(new CTCPCltSocket(CSocket::ASYNC));

			pOutSocket->Connect(pConfig->m_strDstHost, pConfig->m_nDstPort);

			if (App.m_bTraceConns)
				Trace(TXT("Opened TCP connection %d to server %s:%d"), m_nInstance, pOutSocket->Host().c_str(), pOutSocket->Port());

			// Attach event handler.
			pInpSocket->AddClientListener(this);
			pOutSocket->AddClientListener(this);

			CTCPSockPairPtr pSockPair(new CTCPSockPair(pConfig, m_nInstance++, pInpSocket, pOutSocket));
			
			// Add socket pair to collection.
			m_aoTCPCltSocks.push_back(pSockPair);

			// Add socket pair to map.
			m_oSockMap.insert(std::make_pair(pInpSocket.get(), pSockPair));
			m_oSockMap.insert(std::make_pair(pOutSocket.get(), pSockPair));
		}
	}
	catch (const CSocketException& e)
	{
		Trace(TXT("Failed to accept client connection on port %d  - %s"), pSvrSocket->Port(), e.twhat());
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

	CSockPairPtr pPair = it->second;

	try
	{
		// TCP socket pair?
		if (pPair->m_pConfig->m_nType == SOCK_STREAM)
		{
			CTCPSockPairPtr pSockPair = Core::static_ptr_cast<CTCPSockPair>(pPair);

			size_t nAvail, nRead;

			// Check client side socket.
			if ( (pSocket == pSockPair->m_pInpSocket.get())
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
			if ( (pSocket == pSockPair->m_pOutSocket.get())
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
			CUDPSockPairPtr pSockPair = Core::static_ptr_cast<CUDPSockPair>(pPair);

			size_t nAvail, nRead;

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
	catch (const CSocketException& e)
	{
		Trace(TXT("Failed to forward packets on connection %d - %s"), pPair->m_nInstance, e.twhat());
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

	CSockPairPtr pSockPair = it->second;

	ASSERT(pSockPair.get() != nullptr);
	ASSERT(pSockPair->m_pConfig->m_nType == SOCK_STREAM);

	// Must be a TCP pair.
	CTCPSockPairPtr pTCPSockPair = Core::static_ptr_cast<CTCPSockPair>(pSockPair);

	if (App.m_bTraceConns)
		Trace(TXT("Connection %d closed on port %d"), pSockPair->m_nInstance, pSockPair->m_pConfig->m_nSrcPort);

	// Detach event handler.
	pTCPSockPair->m_pInpSocket->RemoveClientListener(this);
	pTCPSockPair->m_pOutSocket->RemoveClientListener(this);

	// Close both sockets.
	pTCPSockPair->m_pInpSocket->Close();
	pTCPSockPair->m_pOutSocket->Close();

	// Remove socket pair from map.
	m_oSockMap.erase(m_oSockMap.find(pTCPSockPair->m_pInpSocket.get()));
	m_oSockMap.erase(m_oSockMap.find(pTCPSockPair->m_pOutSocket.get()));
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

	CSockPairPtr pSockPair = it->second;

	ASSERT(pSockPair.get() != nullptr);

	Trace(TXT("%s Error on connection %d: %s"), CSocket::AsyncEventStr(nEvent).c_str(), pSockPair->m_nInstance, CWinSock::ErrorToSymbol(nError).c_str());
}
