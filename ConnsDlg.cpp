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

	DEFINE_GRAVITY_TABLE
		CTRLGRAV(IDC_SOCKETS, LEFT_EDGE,  TOP_EDGE,    RIGHT_EDGE, BOTTOM_EDGE)
		CTRLGRAV(IDCANCEL,    RIGHT_EDGE, BOTTOM_EDGE, RIGHT_EDGE, BOTTOM_EDGE)
	END_GRAVITY_TABLE
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
	m_lvSocks.InsertColumn(STATUS,   "Status", 50, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(PROTOCOL, "Proto",  50, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(HOST,     "Host",  150, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(PORT,     "Port",   50, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(SENT,     "# Sent", 60, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(RECV,     "# Recv", 60, LVCFMT_LEFT);

	// Add all TCP sockets...
	for (uint i = 0; i < App.m_aoTCPCltSocks.size(); ++i)
	{
		CTCPSockPair* pPair = App.m_aoTCPCltSocks[i];

		CString strConn = CStrCvt::FormatInt(pPair->m_nInstance);

		bool bOpen = (pPair->m_pInpSocket->IsOpen() && pPair->m_pOutSocket->IsOpen());

		m_lvSocks.InsertItem(i,           strConn);
		m_lvSocks.ItemText  (i, STATUS,   (bOpen) ? "Open" : "Closed");
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
