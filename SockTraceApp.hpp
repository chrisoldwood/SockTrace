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

// Template shorthands.
typedef TPtrArray<CSockConfig>     CSockConfigs;
typedef TPtrArray<CTCPSvrSocket>   CTCPSvrSockets;
typedef TPtrArray<CTCPSockPair>    CTCPCltSockets;
typedef TPtrArray<CUDPSockPair>    CUDPSvrSockets;
typedef TMap<CSocket*, CSockPair*> CSocketMap;

/******************************************************************************
** 
** The application class.
**
*******************************************************************************
*/

class CSockTraceApp : public CApp, public IClientSocketListener, public IServerSocketListener
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

	uint			m_nInstance;		// The instance counter.

	CIniFile		m_oIniFile;			// .INI FIle
	CRect			m_rcLastPos;		// Main window position.

	CSockConfigs	m_aoConfigs;		// The socket configurations.
	bool			m_bCfgModified;		// Config modified?

	CTCPSvrSockets	m_aoTCPSvrSocks;	// The TCP listener sockets.
	CTCPCltSockets	m_aoTCPCltSocks;	// The TCP client <-> server sockets.
	CUDPSvrSockets	m_aoUDPSvrSocks;	// The UDP client <-> server sockets.
	CSocketMap		m_oSockMap;			// The socket -> socket pair map.

	bool			m_bTrayIcon;		// Show system tray indicator?
	bool			m_bMinToTray;		// Minimise to system tray?

	bool			m_bTraceConns;		// Trace connections?
	bool			m_bTraceData;		// Trace data transfer?
	bool			m_bTraceToWindow;	// Trace output to window?
	int				m_nTraceLines;		// Trace lines in window.
	bool			m_bTraceToFile;		// Trace output to file?
	CString			m_strTraceFile;		// Trace filename.

	CPath			m_strTracePath;		// The trace file full path.

	//
	// Methods.
	//
	void OpenSockets();
	void CloseSockets();

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
	// Config methods.
	//
	void LoadConfig();
	void SaveConfig();

	//
	// Socket methods.
	//
	CSockConfig* FindConfig(int nType, uint nPort) const;

	//
	// Constants.
	//
	static const char* INI_FILE_VER;

	static const bool  DEF_TRAY_ICON;
	static const bool  DEF_MIN_TO_TRAY;
	static const bool  DEF_TRACE_CONNS;
	static const bool  DEF_TRACE_DATA;
	static const bool  DEF_TRACE_TO_WINDOW;
	static const int   DEF_TRACE_LINES;
	static const bool  DEF_TRACE_TO_FILE;
	static const char* DEF_TRACE_FILE;

	//
	// Socket event handlers.
	//
	virtual void OnAcceptReady(CTCPSvrSocket* pSocket);
	virtual void OnReadReady(CSocket* pSocket);
	virtual void OnClosed(CSocket* pSocket, int nReason);
	virtual void OnError(CSocket* pSocket, int nEvent, int nError);
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
