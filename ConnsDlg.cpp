/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		CONNSDLG.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CConnsDlg class definition.
**
*******************************************************************************
*/

#include "AppHeaders.hpp"
#include "ConnsDlg.hpp"

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

CConnsDlg::CConnsDlg()
	: CDialog(IDD_CONNS)
{
	DEFINE_CTRL_TABLE
		CTRL(IDC_SOCKETS, &m_lvSocks)
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

void CConnsDlg::OnInitDialog()
{
	// Set listview style.
	m_lvSocks.FullRowSelect(true);

	// Create listview columns.
	m_lvSocks.InsertColumn(INSTANCE, "Conn",   50, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(PROTOCOL, "Proto",  50, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(HOST,     "Host",  150, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(PORT,     "Port",   50, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(SENT,     "# Sent", 60, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(RECV,     "# Recv", 60, LVCFMT_LEFT);

	// Add all TCP sockets...
	for (int i = 0; i < App.m_aoTCPCltSocks.Size(); ++i)
	{
		CTCPSockPair* pPair = App.m_aoTCPCltSocks[i];

		CString strConn = CStrCvt::FormatInt(pPair->m_nInstance);

		if (pPair->m_pInpSocket->IsOpen() && pPair->m_pOutSocket->IsOpen())
			strConn += '*';

		m_lvSocks.InsertItem(i,           strConn);
		m_lvSocks.ItemText  (i, PROTOCOL, pPair->m_pConfig->m_strType);
		m_lvSocks.ItemText  (i, HOST,     pPair->m_pConfig->m_strDstHost);
		m_lvSocks.ItemText  (i, PORT,     CStrCvt::FormatInt(pPair->m_pConfig->m_nDstPort));
		m_lvSocks.ItemText  (i, SENT,     CStrCvt::FormatInt(pPair->m_nBytesSent));
		m_lvSocks.ItemText  (i, RECV,     CStrCvt::FormatInt(pPair->m_nBytesRecv));
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

bool CConnsDlg::OnOk()
{
	return true;
}
