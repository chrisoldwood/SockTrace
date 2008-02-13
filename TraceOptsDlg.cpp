/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		TRACEOPTSDLG.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CTraceOptsDlg class definition.
**
*******************************************************************************
*/

#include "Common.hpp"
#include "TraceOptsDlg.hpp"
#include "SockTraceApp.hpp"
#include <WCL/StrCvt.hpp>

/******************************************************************************
** Method:		Default constructor.
**
** Description:	.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

CTraceOptsDlg::CTraceOptsDlg()
	: CDialog(IDD_TRACE_OPTIONS)
{
	DEFINE_CTRL_TABLE
		CTRL(IDC_CONNECTIONS,	&m_ckTraceConns)
		CTRL(IDC_DATA,			&m_ckTraceData)
		CTRL(IDC_WINDOW,		&m_ckTraceToWindow)
		CTRL(IDC_MAX_LINES,		&m_ebTraceLines)
		CTRL(IDC_FILE,			&m_ckTraceToFile)
		CTRL(IDC_FILE_NAME,		&m_ebTraceFile)
	END_CTRL_TABLE

	DEFINE_CTRLMSG_TABLE
	END_CTRLMSG_TABLE
}

/******************************************************************************
** Method:		OnInitDialog()
**
** Description:	Initialise the dialog.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CTraceOptsDlg::OnInitDialog()
{
	m_ckTraceConns.Check(App.m_bTraceConns);
	m_ckTraceData.Check(App.m_bTraceData);
	m_ckTraceToWindow.Check(App.m_bTraceToWindow);
	m_ebTraceLines.Text(CStrCvt::FormatInt(App.m_nTraceLines));
	m_ckTraceToFile.Check(App.m_bTraceToFile);
	m_ebTraceFile.Text(App.m_strTraceFile);
}

/******************************************************************************
** Method:		OnOk()
**
** Description:	The OK button was pressed.
**
** Parameters:	None.
**
** Returns:		true or false.
**
*******************************************************************************
*/

bool CTraceOptsDlg::OnOk()
{
	if ( (m_ckTraceToFile.IsChecked()) && (m_ebTraceFile.TextLength() == 0) )
	{
		AlertMsg(TXT("Please supply the trace filename."));
		m_ebTraceFile.Focus();
		return false;
	}

	// Get control values.
	App.m_bTraceConns    = m_ckTraceConns.IsChecked();
	App.m_bTraceData     = m_ckTraceData.IsChecked();
	App.m_bTraceToWindow = m_ckTraceToWindow.IsChecked();
	App.m_nTraceLines    = CStrCvt::ParseUInt(m_ebTraceLines.Text());
	App.m_bTraceToFile   = m_ckTraceToFile.IsChecked();
	App.m_strTraceFile   = m_ebTraceFile.Text();

	// Mark config dirty.
	App.m_bCfgModified = true;

	// Update full path to trace file.
	App.m_strTracePath = CPath(CPath::ApplicationDir(), App.m_strTraceFile);

	return true;
}
