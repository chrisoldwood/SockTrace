/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKCFGDLG.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CSockCfgDlg class definition.
**
*******************************************************************************
*/

#include "AppHeaders.hpp"
#include "SockCfgDlg.hpp"

#ifdef _DEBUG
// For memory leak detection.
#define new DBGCRT_NEW
#endif

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

CSockCfgDlg::CSockCfgDlg()
	: CDialog(IDD_SOCK_CONFIG)
	, m_ebSrcPort(false, 5)
	, m_ebDstPort(false, 5)
{
	DEFINE_CTRL_TABLE
		CTRL(IDC_PROTOCOL,	&m_cbProtocol)
		CTRL(IDC_SRC_PORT,	&m_ebSrcPort )
		CTRL(IDC_DST_HOST,	&m_ebDstHost )
		CTRL(IDC_DST_PORT,	&m_ebDstPort )
		CTRL(IDC_SEND_FILE,	&m_ebSendFile)
		CTRL(IDC_RECV_FILE,	&m_ebRecvFile)
	END_CTRL_TABLE

	DEFINE_CTRLMSG_TABLE
		CMD_CTRLMSG(IDC_CHECK, BN_CLICKED, OnCheckHost)
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

void CSockCfgDlg::OnInitDialog()
{
	// Load protocol combo;
	m_cbProtocol.Add("TCP");
	m_cbProtocol.Add("UDP");

	// If editing load current config.
	if (m_oConfig.m_strType != "")
	{
		m_cbProtocol.CurSel(m_cbProtocol.FindExact(m_oConfig.m_strType));
		m_ebSrcPort.IntValue(m_oConfig.m_nSrcPort);
		m_ebDstHost.Text(m_oConfig.m_strDstHost);
		m_ebDstPort.IntValue(m_oConfig.m_nDstPort);
		m_ebSendFile.Text(m_oConfig.m_strSendFile);
		m_ebRecvFile.Text(m_oConfig.m_strRecvFile);
	}
	// Creating, load defaults.
	else
	{
		m_cbProtocol.CurSel(m_cbProtocol.FindExact("TCP"));
		m_ebSendFile.Text("%port%_%id%_%dir%.dat");
		m_ebRecvFile.Text("%port%_%id%_%dir%.dat");
	}
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

bool CSockCfgDlg::OnOk()
{
	// Validate setings.
	if (!Validate(m_ebSrcPort, "Source Port"))
		return false;

	if (!Validate(m_ebDstHost, "Destination Host"))
		return false;

	if (!Validate(m_ebDstPort, "Destination Port"))
		return false;

	if (!Validate(m_ebSendFile, "Send Filename"))
		return false;

	if (!Validate(m_ebRecvFile, "Receive Filename"))
		return false;

	// Extract new settings.
	m_oConfig.m_strType     = m_cbProtocol.Text();
	m_oConfig.m_nSrcPort    = m_ebSrcPort.IntValue();
	m_oConfig.m_strDstHost  = m_ebDstHost.Text();
	m_oConfig.m_nDstPort    = m_ebDstPort.IntValue();
	m_oConfig.m_strSendFile = m_ebSendFile.Text();
	m_oConfig.m_strRecvFile = m_ebRecvFile.Text();

	// Get protocol type.
	m_oConfig.m_nType = (m_oConfig.m_strType == "TCP") ? SOCK_STREAM : SOCK_DGRAM;

	return true;
}

/******************************************************************************
** Method:		OnCheckHost()
**
** Description:	Checks that the destination host resolves to an address.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockCfgDlg::OnCheckHost()
{
	CBusyCursor oHourGlass;

	// Ignore, if no host supplied.
	if (m_ebDstHost.TextLength() == 0)
		return;

	CString strHost = m_ebDstHost.Text();

	try
	{
		// Attempt to resolve it.
		CString strIP = CSocket::ResolveStr(strHost);

		NotifyMsg("Resolved %s to %s", strHost, strIP);
	}
	catch (CSocketException& /*e*/)
	{
		AlertMsg("Failed to resolve: %s", strHost);
	}
}

/******************************************************************************
** Method:		Validate()
**
** Description:	Check that the user has entered a value for the setting.
**
** Parameters:	oEditBox	The edit box to check.
**				pszSetting	The name for the setting.
**
** Returns:		true or false.
**
*******************************************************************************
*/

bool CSockCfgDlg::Validate(CEditBox& oEditBox, const char* pszSetting)
{
	ASSERT(pszSetting != NULL);

	// Edit box empty?
	if (oEditBox.TextLength() == 0)
	{
		AlertMsg("Please enter the %s.", pszSetting);
		oEditBox.Focus();
		return false;
	}

	return true;
}
