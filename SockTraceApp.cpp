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
const char* CSockTraceApp::VERSION      = "v1.0 [Debug]";
#else
const char* CSockTraceApp::VERSION      = "v1.0";
#endif

const char* CSockTraceApp::INI_FILE_VER  = "1.0";
const uint  CSockTraceApp::BG_TIMER_FREQ =  1;

const char* CSockTraceApp::DEF_SEND_FILE = "Send.dat";
const char* CSockTraceApp::DEF_RECV_FILE = "Recv.dat";

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
	, m_nTimerID(0)
	, m_nSrcPort(0)
	, m_nDstPort(0)
	, m_pTCPLstSock(NULL)
	, m_pTCPInpSocket(NULL)
	, m_pTCPOutSocket(NULL)
	, m_pUDPInpSocket(NULL)
	, m_pUDPOutSocket(NULL)
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

	// Create the full log file paths.
	m_strSendPath = CPath(CPath::ApplicationDir(), m_strSendFile);
	m_strRecvPath = CPath(CPath::ApplicationDir(), m_strRecvFile);

	// Clear the log files.
	try
	{
		CFile fSendFile, fRecvFile;

		fSendFile.Create(m_strSendPath);
		fSendFile.Close();

		fRecvFile.Create(m_strRecvPath);
		fRecvFile.Close();
	}
	catch (CFileException& e)
	{
		AlertMsg("Failed to truncate log file:\n\n%s", e.ErrorText());
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

	try
	{
		CBusyCursor oCursor;

		// If set, resolve the source hostname.
		if ( (m_strSrcHost != "") && (!CSocket::IsAddress(m_strSrcHost)) )
		{
			in_addr addr = CSocket::Resolve(m_strSrcHost);

			Trace("Resolved source hostname: %s to %s", m_strSrcHost, inet_ntoa(addr));
		}

		// If set, resolve the destination hostname.
		if ( (m_strDstHost != "") && (!CSocket::IsAddress(m_strDstHost)) )
		{
			in_addr addr = CSocket::Resolve(m_strDstHost);

			Trace("Resolved destination hostname: %s to %s", m_strDstHost, inet_ntoa(addr));
		}
	}
	catch (CSocketException& e)
	{
		AlertMsg("Failed to resolve hostname:\n\n%s", e.ErrorText());
		return false;
	}

	try
	{
		Trace("Opening %s server socket on port: %d", m_strType, m_nSrcPort);

		ASSERT(m_nSrcPort != m_nDstPort);

		if (m_nType == SOCK_STREAM)
		{
			// Create the listening socket.
			m_pTCPLstSock = new CTCPSvrSocket;
			m_pTCPLstSock->Listen(m_nSrcPort, 1);
		}
		else
		{
			// Create both UDP server sockets.
			m_pUDPInpSocket = new CUDPSvrSocket;
			m_pUDPInpSocket->Listen(m_nSrcPort);

			m_pUDPOutSocket = new CUDPSvrSocket; 
			m_pUDPOutSocket->Listen(m_nDstPort);
		}
	}
	catch (CSocketException& e)
	{
		AlertMsg("Failed to create server socket:\n\n%s", e.ErrorText());
		return false;
	}

	// Start the background timer.
	m_nTimerID = StartTimer(BG_TIMER_FREQ);

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
	// Cleanup the sockets.
	delete m_pTCPOutSocket;
	delete m_pTCPInpSocket;
	delete m_pTCPLstSock;
	delete m_pUDPInpSocket;
	delete m_pUDPOutSocket;

	// Stop the background timer.
	StopTimer(m_nTimerID);

	// Terminate WinSock.
	CWinSock::Cleanup();

	// Save settings.
	SaveConfig();

	return true;
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

	// Read the socket settings.
	m_strType    = m_oIniFile.ReadString("Socket", "Type",     "TCP");
	m_strSrcHost = m_oIniFile.ReadString("Socket", "SrcHost",  m_strSrcHost);
	m_nSrcPort   = m_oIniFile.ReadInt   ("Socket", "SrcPort",  m_nSrcPort  );
	m_strDstHost = m_oIniFile.ReadString("Socket", "DstHost",  m_strDstHost);
	m_nDstPort   = m_oIniFile.ReadInt   ("Socket", "DstPort",  m_nDstPort  );

	// Get the WinSock socket type.
	m_nType = (m_strType == "TCP") ? SOCK_STREAM : SOCK_DGRAM;

	// Read the log filenames.
	m_strSendFile = m_oIniFile.ReadString("Logs", "Send",  DEF_SEND_FILE);
	m_strRecvFile = m_oIniFile.ReadString("Logs", "Recv",  DEF_RECV_FILE);

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
	m_oIniFile.WriteInt   ("Socket", "SrcPort", m_nSrcPort  );
	m_oIniFile.WriteString("Socket", "DstHost", m_strDstHost);
	m_oIniFile.WriteInt   ("Socket", "DstPort", m_nDstPort  );

	// Write the log filenames.
	m_oIniFile.ReadString("Logs", "Send",  m_strSendFile);
	m_oIniFile.ReadString("Logs", "Recv",  m_strRecvFile);

	// Write the window pos and size.
	m_oIniFile.WriteInt("UI", "Left",   m_rcLastPos.left  );
	m_oIniFile.WriteInt("UI", "Top",    m_rcLastPos.top   );
	m_oIniFile.WriteInt("UI", "Right",  m_rcLastPos.right );
	m_oIniFile.WriteInt("UI", "Bottom", m_rcLastPos.bottom);
}

/******************************************************************************
** Method:		OnTimer()
**
** Description:	The timer has gone off, process background tasks.
**				NB: Re-entrancy can be caused when performing DDE requests.
**
** Parameters:	nTimerID	The timer ID.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::OnTimer(uint nTimerID)
{
	try
	{
		ASSERT(nTimerID == m_nTimerID);

		// Process TCP client connections.
		if (m_nType == SOCK_STREAM)
			HandleConnects();

		// Process data transfer.
		if (m_nType == SOCK_STREAM)
			HandleTCPPackets();
		else
			HandleUDPPackets();

		// Process TCP client disconnections.
		if (m_nType == SOCK_STREAM)
			HandleDisconnects();
	}
	catch (CSocketException& e)
	{
		StopTimer(m_nTimerID);

		FatalMsg("Failed to link client and server: %s", e.ErrorText());

		m_AppWnd.Close();
	}
	catch (CFileException& e)
	{
		StopTimer(m_nTimerID);

		FatalMsg("Failed to log socket data: %s", e.ErrorText());

		m_AppWnd.Close();
	}
	catch (...)
	{
		StopTimer(m_nTimerID);

		FatalMsg("Unhandled Exception.");

		m_AppWnd.Close();
	}
}

/******************************************************************************
** Method:		HandleConnects()
**
** Description:	Check for a client connecting to the server.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
** Exceptions:	CSocketException.
**
*******************************************************************************
*/

void CSockTraceApp::HandleConnects()
{
	ASSERT(m_pTCPLstSock != NULL);

	// No connection and one waiting?
	if ( (m_pTCPInpSocket == NULL) && (m_pTCPLstSock->CanAccept()) )
	{
		// Accept the connection.
		m_pTCPInpSocket = m_pTCPLstSock->Accept();

		ASSERT(m_pTCPInpSocket != NULL);

		Trace("Connection accepted from: %s", m_pTCPInpSocket->PeerAddress());

		ASSERT(m_pTCPOutSocket == NULL);

		// Open socket to destination.
		m_pTCPOutSocket = new CTCPCltSocket();
		m_pTCPOutSocket->Connect(m_strDstHost, m_nDstPort);

		Trace("Opened connection to server: %s", m_strSrcHost);
	}
}

/******************************************************************************
** Method:		HandleTCPPackets()
**
** Description:	Transfer data between the client and server sockets and write
**				any data to the log file as well.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::HandleTCPPackets()
{
	// No connection?
	if ( (m_pTCPInpSocket == NULL) || (m_pTCPOutSocket == NULL) )
		return;

	try
	{
		int nAvail, nRead;

		// Check client side socket.
		if ((nAvail = m_pTCPInpSocket->Available()) > 0)
		{
			CBuffer oBuffer(nAvail);

			// Read data from the client socket.
			if ((nRead = m_pTCPInpSocket->Recv(oBuffer)) > 0)
			{
				Trace("Forwarded %d bytes to the server", nRead);

				// Send down the server socket.
				m_pTCPOutSocket->Send(oBuffer.Buffer(), nRead);

				// Log the data sent by the client.
				LogData(m_strSendPath, oBuffer.Buffer(), nRead);
			}
		}

		// Check server side socket.
		if ((nAvail = m_pTCPOutSocket->Available()) > 0)
		{
			CBuffer oBuffer(nAvail);

			// Read data from the server socket.
			if ((nRead = m_pTCPOutSocket->Recv(oBuffer)) > 0)
			{
				Trace("Forwarded %d bytes to the client", nRead);

				// Send down the client socket.
				m_pTCPInpSocket->Send(oBuffer.Buffer(), nRead);

				// Log the data received by the client.
				LogData(m_strRecvPath, oBuffer.Buffer(), nRead);
			}
		}
	}
	catch (CSocketException& e)
	{
		if (e.ErrorCode() == CSocketException::E_DISCONNECTED)
			Trace("Connection closed");
		else
			Trace("Failed to forward packets: %s", e.ErrorText());

		m_pTCPInpSocket->Close();
		m_pTCPOutSocket->Close();
	}
}

/******************************************************************************
** Method:		HandleUDPPackets()
**
** Description:	Transfer data between the client and server sockets and write
**				any data to the log file as well.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::HandleUDPPackets()
{
	ASSERT((m_pUDPInpSocket != NULL) && (m_pUDPOutSocket != NULL));

	try
	{
		int     nAvail, nRead, nPort;
		in_addr oAddr;

		// Check client side socket.
		if ((nAvail = m_pUDPInpSocket->Available()) > 0)
		{
			CBuffer oBuffer(nAvail);

			// Read data from the client socket.
			if ((nRead = m_pUDPInpSocket->RecvFrom(oBuffer, oAddr, nPort)) > 0)
			{
				Trace("Forwarded %d bytes to the server", nRead);

				// Send down the server socket.
//				m_pTCPOutSocket->Send(oBuffer.Buffer(), nRead);

				// Log the data sent by the client.
				LogData(m_strSendPath, oBuffer.Buffer(), nRead);
			}
		}

		// Check server side socket.
		if ((nAvail = m_pUDPOutSocket->Available()) > 0)
		{
			CBuffer oBuffer(nAvail);

			// Read data from the server socket.
			if ((nRead = m_pUDPOutSocket->RecvFrom(oBuffer, oAddr, nPort)) > 0)
			{
				Trace("Forwarded %d bytes to the client", nRead);

				// Send down the client socket.
//				m_pTCPInpSocket->Send(oBuffer.Buffer(), nRead);

				// Log the data received by the client.
				LogData(m_strRecvPath, oBuffer.Buffer(), nRead);
			}
		}
	}
	catch (CSocketException& e)
	{
		Trace("Failed to forward packets: %s", e.ErrorText());
	}
}

/******************************************************************************
** Method:		HandleDisconnects()
**
** Description:	Cleanup if either the client or server sockets have been closed.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockTraceApp::HandleDisconnects()
{
	ASSERT(m_pTCPLstSock != NULL);

	// No connection?
	if ( (m_pTCPInpSocket == NULL) || (m_pTCPOutSocket == NULL) )
		return;
	
	// Either socket closed?
	if ( (!m_pTCPInpSocket->IsOpen()) || (!m_pTCPOutSocket->IsOpen()) )
	{
		// Free connection.
		delete m_pTCPInpSocket;
		delete m_pTCPOutSocket;

		// Reset members.
		m_pTCPInpSocket  = NULL;
		m_pTCPOutSocket = NULL;
	}
}
