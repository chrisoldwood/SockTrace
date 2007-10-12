/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		APPCMDS.CPP
** COMPONENT:	The Application.
** DESCRIPTION:	CAppCmds class definition.
**
*******************************************************************************
*/

#include "AppHeaders.hpp"
#include "ConnsDlg.hpp"
#include "TraceOptsDlg.hpp"
#include "SockOptsDlg.hpp"
#include "AboutDlg.hpp"
#include <Legacy/STLUtils.hpp>

#ifdef _DEBUG
// For memory leak detection.
#define new DBGCRT_NEW
#endif

/******************************************************************************
** Method:		Constructor.
**
** Description:	.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

CAppCmds::CAppCmds()
{
	// Define the command table.
	DEFINE_CMD_TABLE
		// File menu.
		CMD_ENTRY(ID_FILE_EXIT,			OnFileExit,			NULL,	-1)
		// View menu.
		CMD_ENTRY(ID_VIEW_CONNS,		OnViewConnections,	NULL,	-1)
		CMD_ENTRY(ID_VIEW_CLEAR_TRACE,	OnViewClearTrace,	NULL,	-1)
		// Tools menu.
		CMD_ENTRY(ID_TOOLS_HOSTS,		OnToolsEditHosts,	NULL,	-1)
		// Options menu.
		CMD_ENTRY(ID_OPTIONS_GENERAL,	OnOptionsGeneral,	NULL,	-1)
		CMD_ENTRY(ID_OPTIONS_TRACE,		OnOptionsTrace,		NULL,	-1)
		CMD_ENTRY(ID_OPTIONS_SOCKET,	OnOptionsSocket,	NULL,	-1)
		// Help menu.
		CMD_ENTRY(ID_HELP_ABOUT,		OnHelpAbout,		NULL,	10)
	END_CMD_TABLE
}

/******************************************************************************
** Method:		Destructor.
**
** Description:	.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

CAppCmds::~CAppCmds()
{
}

/******************************************************************************
** Method:		OnFileExit()
**
** Description:	Terminate the app.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CAppCmds::OnFileExit()
{
	App.m_AppWnd.Close();
}

/******************************************************************************
** Method:		OnViewConnections()
**
** Description:	View the list of open sockets.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CAppCmds::OnViewConnections()
{
	CConnsDlg Dlg;

	Dlg.RunModal(App.m_rMainWnd);
}

/******************************************************************************
** Method:		OnViewClearTrace()
**
** Description:	Clear the trace window.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CAppCmds::OnViewClearTrace()
{
	App.m_AppWnd.m_AppDlg.Clear();
}

/******************************************************************************
** Method:		OnToolsEditHosts()
**
** Description:	Edit the HOSTS file.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CAppCmds::OnToolsEditHosts()
{
	// Get the full path to the HOSTS file.
	CPath   strHOSTS(CPath::SystemDir(), "drivers\\etc\\HOSTS");

	CString strCmdLine("NOTEPAD.EXE ");

	strCmdLine += strHOSTS;

	// Launch NotePad to edit the file.
	uint nResult = ::WinExec(strCmdLine, SW_SHOW);

	// Report any error.
	if (nResult <= 31)
		App.AlertMsg("Failed to execute:\n\n%s\n\nError code: %d", strCmdLine, nResult);
}

/******************************************************************************
** Method:		OnOptionsGeneral()
**
** Description:	Configure the general options.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CAppCmds::OnOptionsGeneral()
{
}

/******************************************************************************
** Method:		OnOptionsTrace()
**
** Description:	Configure the trace options.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CAppCmds::OnOptionsTrace()
{
	CTraceOptsDlg Dlg;

	Dlg.RunModal(App.m_rMainWnd);
}

/******************************************************************************
** Method:		OnOptionsSocket()
**
** Description:	Configure the socket settings.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CAppCmds::OnOptionsSocket()
{
	bool bOpen = false;

	// Check if there are any open connections.
	for (uint i = 0; i < App.m_aoTCPCltSocks.size(); ++i)
	{
		CTCPSockPair* pPair = App.m_aoTCPCltSocks[i];

		if (pPair->m_pInpSocket->IsOpen() && pPair->m_pOutSocket->IsOpen())
			bOpen = true;
	}

	const char* pszMsg = "WARNING: There are open connections.\n"
						 "Any changes will cause these to be dropped.\n\n"
						 "Do you want to continue?";

	// Query user to continue.
	if ( (bOpen) && (App.QueryMsg(pszMsg) != IDYES) )
		return;

	CSockOptsDlg Dlg;

	DeepCopy(App.m_aoConfigs, Dlg.m_aoConfigs);

	// Show sockets config dialog.
	if ( (Dlg.RunModal(App.m_rMainWnd) == IDOK) && (Dlg.m_bModified) )
	{
		// Close all current sockets.
		App.CloseSockets();

		DeleteAll(App.m_aoConfigs);
		DeepCopy(Dlg.m_aoConfigs, App.m_aoConfigs);

		App.m_bCfgModified = true;

		// Start listening again...
		App.OpenSockets();
	}
}

/******************************************************************************
** Method:		OnHelpAbout()
**
** Description:	Show the about dialog.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CAppCmds::OnHelpAbout()
{
	CAboutDlg Dlg;

	Dlg.RunModal(App.m_rMainWnd);
}

/******************************************************************************
** Method:		OnUI...()
**
** Description:	UI update handlers.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/
