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

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/App.hpp>
#include "AppWnd.hpp"
#include "AppCmds.hpp"
#include <NCL/IClientSocketListener.hpp>
#include <NCL/IServerSocketListener.hpp>
#include "SockConfig.hpp"

// Forward declarations.
class CSockConfig;
class CTCPSvrSocket;
class CTCPCltSocket;
class CTCPSockPair;
class CUDPSockPair;
class CSocket;
class CSockPair;

// Smart-pointer types.
typedef Core::SharedPtr<CSocket> CSocketPtr;
typedef Core::SharedPtr<CTCPSvrSocket> CTCPSvrSocketPtr;
typedef Core::SharedPtr<CTCPCltSocket> CTCPCltSocketPtr;
typedef Core::SharedPtr<CSockPair> CSockPairPtr;
typedef Core::SharedPtr<CTCPSockPair> CTCPSockPairPtr;
typedef Core::SharedPtr<CUDPSockPair> CUDPSockPairPtr;

// Template shorthands.
typedef std::vector<CTCPSvrSocketPtr> CTCPSvrSockets;
typedef std::vector<CTCPSockPairPtr>  CTCPCltSockets;
typedef std::vector<CUDPSockPairPtr>  CUDPSvrSockets;
typedef std::map<CSocket*, CSockPairPtr> CSocketMap;

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
	uint			m_nTraceLines;		// Trace lines in window.
	bool			m_bTraceToFile;		// Trace output to file?
	CString			m_strTraceFile;		// Trace filename.

	CPath			m_appDataFolder;	//!< The application data folder.
	CPath			m_strTracePath;		// The trace file full path.

	//
	// Methods.
	//
	void OpenSockets();
	void CloseSockets();

	void Trace(const tchar* pszMsg, ...);
	void LogData(CPath& strFileName, const void* pvData, size_t nLength);

protected:
	//
	// Startup and Shutdown template methods.
	//
	virtual	bool OnOpen();
	virtual	bool OnClose();

	//
	// Config methods.
	//
	void loadConfig();
	void saveConfig();

	//
	// Socket methods.
	//
	CSockConfigPtr FindConfig(int nType, uint nPort) const;

	//
	// Constants.
	//
	static const bool  DEF_TRAY_ICON;
	static const bool  DEF_MIN_TO_TRAY;
	static const bool  DEF_TRACE_CONNS;
	static const bool  DEF_TRACE_DATA;
	static const bool  DEF_TRACE_TO_WINDOW;
	static const uint  DEF_TRACE_LINES;
	static const bool  DEF_TRACE_TO_FILE;
	static const tchar* DEF_TRACE_FILE;

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
