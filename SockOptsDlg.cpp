/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKOPTSDLG.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CSockOptsDlg class definition.
**
*******************************************************************************
*/

#include "AppHeaders.hpp"
#include "SockOptsDlg.hpp"
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

CSockOptsDlg::CSockOptsDlg()
	: CDialog(IDD_SOCK_OPTIONS)
	, m_bModified(false)
{
	DEFINE_CTRL_TABLE
		CTRL(IDC_SOCKETS,	&m_lvSocks)
	END_CTRL_TABLE

	DEFINE_CTRLMSG_TABLE
		CMD_CTRLMSG(IDC_ADD,    BN_CLICKED, OnAdd   )
		CMD_CTRLMSG(IDC_EDIT,   BN_CLICKED, OnEdit  )
		CMD_CTRLMSG(IDC_REMOVE, BN_CLICKED, OnRemove)
	END_CTRLMSG_TABLE
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

CSockOptsDlg::~CSockOptsDlg()
{
	m_aoConfigs.DeleteAll();
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

void CSockOptsDlg::OnInitDialog()
{
	// Set listview style.
//	m_lvSocks.Font(CFont(ANSI_FIXED_FONT));
	m_lvSocks.FullRowSelect(true);
//	m_lvSocks.GridLines(true);

	// Create listview columns.
	m_lvSocks.InsertColumn(SRC_PORT,     "Port",         70, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(SRC_PROTOCOL, "Protocol",     75, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(DST_HOST,     "Destination", 175, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(DST_PORT,     "Port",         70, LVCFMT_LEFT);

	// Add current socket configs.
	for (int i = 0; i < m_aoConfigs.Size(); ++i)
	{
		CSockConfig* pConfig = m_aoConfigs[i];

		m_lvSocks.InsertItem(i, "");
		m_lvSocks.ItemPtr(i, pConfig);

		UpdateConfig(i, pConfig);
	}

	// Select 1st by default.
	if (m_aoConfigs.Size() > 0)
		m_lvSocks.Select(0);
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

bool CSockOptsDlg::OnOk()
{
	return true;
}

/******************************************************************************
** Method:		OnAdd()
**
** Description:	Add a new socket to the config.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockOptsDlg::OnAdd()
{
	CSockCfgDlg Dlg;

	// Show socket config dialog.
	if (Dlg.RunModal(*this) == IDOK)
	{
		// Add config to collection.
		CSockConfig* pConfig = new CSockConfig;

		*pConfig = Dlg.m_oConfig;

		m_aoConfigs.Add(pConfig);

		// Add config to view.
		int i = m_lvSocks.ItemCount();

		m_lvSocks.InsertItem(i, "");
		m_lvSocks.ItemPtr(i, pConfig);

		UpdateConfig(i, pConfig);

		// Make it the selection.
		m_lvSocks.Select(i);

		m_bModified = true;
	}
}

/******************************************************************************
** Method:		OnEdit()
**
** Description:	Edit the selected socket config.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockOptsDlg::OnEdit()
{
	int nSel = m_lvSocks.Selection();

	// Ignore, if no selection.
	if (nSel == LB_ERR)
		return;

	CSockConfig* pConfig = ItemConfig(nSel);
	CSockCfgDlg  Dlg;

	Dlg.m_oConfig = *pConfig;

	// Show socket config dialog.
	if (Dlg.RunModal(*this) == IDOK)
	{
		// Update config.
		*pConfig = Dlg.m_oConfig;

		// Update view.
		UpdateConfig(nSel, pConfig);

		m_bModified = true;
	}
}

/******************************************************************************
** Method:		OnRemove()
**
** Description:	Remove the selected socket config.
**
** Parameters:	None.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockOptsDlg::OnRemove()
{
	int nSel = m_lvSocks.Selection();

	// Ignore, if no selection.
	if (nSel == LB_ERR)
		return;

	CSockConfig* pConfig = ItemConfig(nSel);

	// Remove from view and collection.
	m_lvSocks.DeleteItem(nSel);
	m_aoConfigs.Delete(m_aoConfigs.Find(pConfig));

	// Update selection.
	m_lvSocks.Select((nSel < m_lvSocks.ItemCount()) ? nSel : nSel-1);

	m_bModified = true;
}

/******************************************************************************
** Method:		UpdateConfig()
**
** Description:	Updates the given config in the view.
**
** Parameters:	nItem	The listview item.
**				pConfig	The config.
**
** Returns:		Nothing.
**
*******************************************************************************
*/

void CSockOptsDlg::UpdateConfig(int nItem, CSockConfig* pConfig)
{
	m_lvSocks.ItemText(nItem, SRC_PORT,     CStrCvt::FormatInt(pConfig->m_nSrcPort));
	m_lvSocks.ItemText(nItem, SRC_PROTOCOL, pConfig->m_strType);
	m_lvSocks.ItemText(nItem, DST_HOST,     pConfig->m_strDstHost);
	m_lvSocks.ItemText(nItem, DST_PORT,     CStrCvt::FormatInt(pConfig->m_nDstPort));
}
