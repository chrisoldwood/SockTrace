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
typedef TPtrArray<CSockConfig>   CSockConfigs;
typedef TPtrArray<CTCPSvrSocket> CTCPSvrSockets;
typedef TPtrArray<CTCPSockPair>  CTCPCltSockets;
typedef TPtrArray<CUDPSockPair>  CUDPSvrSockets;

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
	uint			m_nInstance;		// The instance counter.

	CIniFile		m_oIniFile;			// .INI FIle
	CRect			m_rcLastPos;		// Main window position.

	CSockConfigs	m_aoConfigs;		// The socket configurations.
	CTCPSvrSockets	m_aoTCPSvrSocks;	// The TCP listener sockets.
	CTCPCltSockets	m_aoTCPCltSocks;	// The TCP client <-> server sockets.
	CUDPSvrSockets	m_aoUDPSvrSocks;	// The UDP client <-> server sockets.

	CPath			m_strTraceFile;		// Application trace file.

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
	static const uint  BG_TIMER_FREQ;

	//
	// The backgound timer methods.
	//
	virtual void OnTimer(uint nTimerID);

	//
	// Socket event handlers.
	//
	void HandleTCPConnects();
	void HandleTCPPackets();
	void HandleUDPPackets();
	void HandleTCPDisconnects();
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
