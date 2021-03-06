/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		APPCMDS.CPP
** COMPONENT:	The Application.
** DESCRIPTION:	CAppCmds class definition.
**
*******************************************************************************
*/

#include "Common.hpp"
#include "AppCmds.hpp"
#include "SockTraceApp.hpp"
#include "ConnsDlg.hpp"
#include "TraceOptsDlg.hpp"
#include "SockOptsDlg.hpp"
#include "AboutDlg.hpp"
#include "TCPSockPair.hpp"
#include <Core/AnsiWide.hpp>
#include <Core/Algorithm.hpp>

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

CAppCmds::CAppCmds(CAppWnd& appWnd)
	: CCmdControl(appWnd)
{
	// Define the command table.
	DEFINE_CMD_TABLE
		// File menu.
		CMD_ENTRY(ID_FILE_EXIT,			&CAppCmds::OnFileExit,			nullptr,	-1)
		// View menu.
		CMD_ENTRY(ID_VIEW_CONNS,		&CAppCmds::OnViewConnections,	nullptr,	-1)
		CMD_ENTRY(ID_VIEW_CLEAR_TRACE,	&CAppCmds::OnViewClearTrace,	nullptr,	-1)
		// Tools menu.
		CMD_ENTRY(ID_TOOLS_HOSTS,		&CAppCmds::OnToolsEditHosts,	nullptr,	-1)
		// Options menu.
		CMD_ENTRY(ID_OPTIONS_GENERAL,	&CAppCmds::OnOptionsGeneral,	nullptr,	-1)
		CMD_ENTRY(ID_OPTIONS_TRACE,		&CAppCmds::OnOptionsTrace,		nullptr,	-1)
		CMD_ENTRY(ID_OPTIONS_SOCKET,	&CAppCmds::OnOptionsSocket,		nullptr,	-1)
		// Help menu.
		CMD_ENTRY(ID_HELP_ABOUT,		&CAppCmds::OnHelpAbout,			nullptr,	10)
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
	CPath   strHOSTS(CPath::SystemDir(), TXT("drivers\\etc\\HOSTS"));

	CString strCmdLine(TXT("NOTEPAD.EXE "));

	strCmdLine += strHOSTS;

	// Launch NotePad to edit the file.
	uint nResult = ::WinExec(T2A(strCmdLine), SW_SHOW);

	// Report any error.
	if (nResult <= 31)
		App.AlertMsg(TXT("Failed to execute:\n\n%s\n\nError code: %d"), strCmdLine.c_str(), nResult);
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
		CTCPSockPairPtr pPair = App.m_aoTCPCltSocks[i];

		if (pPair->m_pInpSocket->IsOpen() && pPair->m_pOutSocket->IsOpen())
			bOpen = true;
	}

	const tchar* pszMsg = TXT("WARNING: There are open connections.\n")
						  TXT("Any changes will cause these to be dropped.\n\n")
						  TXT("Do you want to continue?");

	// Query user to continue.
	if ( (bOpen) && (App.QueryMsg(pszMsg) != IDYES) )
		return;

	CSockOptsDlg Dlg;

	Core::deepCopy(App.m_aoConfigs, Dlg.m_aoConfigs);

	// Show sockets config dialog.
	if ( (Dlg.RunModal(App.m_rMainWnd) == IDOK) && (Dlg.m_bModified) )
	{
		// Close all current sockets.
		App.CloseSockets();

		App.m_aoConfigs.clear();
		Core::deepCopy(Dlg.m_aoConfigs, App.m_aoConfigs);

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
	AboutDlg Dlg;

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
