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

	CPath			m_strTraceFile;		// Application trace file.

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

	//
	// Socket event handlers.
	//
	virtual void OnAcceptReady(CTCPSvrSocket* pSocket);
	virtual void OnReadReady(CSocket* pSocket);
	virtual void OnWriteReady(CSocket* pSocket);
	virtual void OnClosed(CSocket* pSocket, int nReason);
//	virtual void OnError(CSocket* pSocket, int nEvent, int nError);
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
