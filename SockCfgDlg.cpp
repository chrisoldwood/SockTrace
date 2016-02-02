/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKCFGDLG.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CSockCfgDlg class definition.
**
*******************************************************************************
*/

#include "Common.hpp"
#include "SockCfgDlg.hpp"
#include <WCL/BusyCursor.hpp>
#include <NCL/Socket.hpp>
#include <NCL/SocketException.hpp>
#include <Core/Algorithm.hpp>

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
		CMD_CTRLMSG(IDC_RESOLVE, BN_CLICKED, &CSockCfgDlg::OnResolveHost)
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
	m_cbProtocol.Add(TXT("TCP"));
	m_cbProtocol.Add(TXT("UDP"));

	// If editing load current config.
	if (m_oConfig.m_strType != TXT(""))
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
		m_cbProtocol.CurSel(m_cbProtocol.FindExact(TXT("TCP")));
		m_ebSendFile.Text(TXT("%port%_%id%_%dir%.dat"));
		m_ebRecvFile.Text(TXT("%port%_%id%_%dir%.dat"));
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
	if (!Validate(m_ebSrcPort, TXT("Source Port")))
		return false;

	if (!Validate(m_ebDstHost, TXT("Destination Host")))
		return false;

	if (!Validate(m_ebDstPort, TXT("Destination Port")))
		return false;

	if (!Validate(m_ebSendFile, TXT("Send Filename")))
		return false;

	if (!Validate(m_ebRecvFile, TXT("Receive Filename")))
		return false;

	// Extract new settings.
	m_oConfig.m_strType     = m_cbProtocol.Text();
	m_oConfig.m_nSrcPort    = m_ebSrcPort.IntValue();
	m_oConfig.m_strDstHost  = m_ebDstHost.Text();
	m_oConfig.m_nDstPort    = m_ebDstPort.IntValue();
	m_oConfig.m_strSendFile = m_ebSendFile.Text();
	m_oConfig.m_strRecvFile = m_ebRecvFile.Text();

	// Get protocol type.
	m_oConfig.m_nType = (m_oConfig.m_strType == TXT("TCP")) ? SOCK_STREAM : SOCK_DGRAM;

	// Port already in use?
	if ( ((m_oConfig.m_nType == SOCK_STREAM) && Core::exists(m_anTCPPorts, m_oConfig.m_nSrcPort))
	  || ((m_oConfig.m_nType == SOCK_DGRAM ) && Core::exists(m_anUDPPorts, m_oConfig.m_nSrcPort)) )
	{
		AlertMsg(TXT("The local port '%d' has already been used."), m_oConfig.m_nSrcPort);
		m_ebSrcPort.Focus();
		return false;
	}

	return true;
}

/******************************************************************************
** Method:		OnResolveHost()
**
** Description:	Resolves a destination hostname to an address.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockCfgDlg::OnResolveHost()
{
	CBusyCursor oHourGlass;

	// Ignore, if no host supplied.
	if (m_ebDstHost.TextLength() == 0)
		return;

	CString strHost = m_ebDstHost.Text();

	try
	{
		// Attempt to resolve it.
		m_ebDstHost.Text(CSocket::ResolveStr(strHost));
	}
	catch (const CSocketException& /*e*/)
	{
		AlertMsg(TXT("Failed to resolve: %s"), strHost);
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

bool CSockCfgDlg::Validate(CEditBox& oEditBox, const tchar* pszSetting)
{
	ASSERT(pszSetting != NULL);

	// Edit box empty?
	if (oEditBox.TextLength() == 0)
	{
		AlertMsg(TXT("Please enter the %s."), pszSetting);
		oEditBox.Focus();
		return false;
	}

	return true;
}
