/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKTRACEAPP.HPP
** COMPONENT:	The Application.
** DESCRIPTION:	The CSockTraceApp class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef SOCKTRACEAPP_HPP
#define SOCKTRACEAPP_HPP

/******************************************************************************
** 
** The application class.
**
*******************************************************************************
*/

class CSockTraceApp : public CApp
{
public:
	//
	// Constructors/Destructor.
	//
	CSockTraceApp();
	~CSockTraceApp();

	//
	// Members.
	//
	CAppWnd			m_AppWnd;			// Main window.
	CAppCmds		m_AppCmds;			// Command handler.

	uint			m_nTimerID;			// The background timer ID.

	CIniFile		m_oIniFile;			// .INI FIle
	CRect			m_rcLastPos;		// Main window position.

	CString			m_strType;			// The socket type.
	int				m_nType;			// The socket type (STREAM/DGRAM).
	CString			m_strSrcHost;		// The source hostname.
	uint			m_nSrcPort;			// The source port number.
	CString			m_strDstHost;		// The destination hostname.
	uint			m_nDstPort;			// The destination port number.

	CString			m_strSendFile;		// The Send log filename.
	CString			m_strRecvFile;		// The Receive log filename.

	CPath			m_strSendPath;		// Path of log file for data sent.
	CPath			m_strRecvPath;		// Path of log file for data recieved.

	CTCPSvrSocket*	m_pTCPLstSock;		// The TCP listening socket.
	CTCPCltSocket*	m_pTCPInpSocket;	// The TCP incoming socket.
	CTCPCltSocket*	m_pTCPOutSocket;	// The TCP outgoing socket.
	CUDPSvrSocket*	m_pUDPInpSocket;	// The UDP incoming socket.
	CUDPSvrSocket*	m_pUDPOutSocket;	// The UDP outgoing socket.

	//
	// Methods.
	//
	void Trace(const char* pszMsg, ...);
	void LogData(CPath& strFileName, const void* pvData, uint nLength);

	//
	// Constants.
	//
	static const char* VERSION;

protected:
	//
	// Startup and Shutdown template methods.
	//
	virtual	bool OnOpen();
	virtual	bool OnClose();

	//
	// Internal methods.
	//
	void LoadConfig();
	void SaveConfig();

	//
	// Constants.
	//
	static const char* INI_FILE_VER;
	static const uint  BG_TIMER_FREQ;
	static const char* DEF_SEND_FILE;
	static const char* DEF_RECV_FILE;

	//
	// The backgound timer methods.
	//
	virtual void OnTimer(uint nTimerID);

	//
	// Socket event handlers.
	//
	void HandleConnects();
	void HandleTCPPackets();
	void HandleUDPPackets();
	void HandleDisconnects();
};

/******************************************************************************
**
** Global variables.
**
*******************************************************************************
*/

// The application object.
extern CSockTraceApp App;

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/


#endif //SOCKTRACEAPP_HPP
