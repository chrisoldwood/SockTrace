/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKOPTSDLG.CPP
** COMPONENT:	The Application
** DESCRIPTION:	CSockOptsDlg class definition.
**
*******************************************************************************
*/

#include "Common.hpp"
#include "SockOptsDlg.hpp"
#include "SockCfgDlg.hpp"
#include <Legacy/STLUtils.hpp>
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

CSockOptsDlg::CSockOptsDlg()
	: CDialog(IDD_SOCK_OPTIONS)
	, m_bModified(false)
{
	DEFINE_CTRL_TABLE
		CTRL(IDC_SOCKETS,	&m_lvSocks)
	END_CTRL_TABLE

	DEFINE_CTRLMSG_TABLE
		CMD_CTRLMSG(IDC_ADD,     BN_CLICKED, &CSockOptsDlg::OnAdd)
		CMD_CTRLMSG(IDC_EDIT,    BN_CLICKED, &CSockOptsDlg::OnEdit)
		CMD_CTRLMSG(IDC_REMOVE,  BN_CLICKED, &CSockOptsDlg::OnRemove)
		NFY_CTRLMSG(IDC_SOCKETS, NM_DBLCLK,  &CSockOptsDlg::OnListDoubleClick)
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
	DeleteAll(m_aoConfigs);
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
	m_lvSocks.FullRowSelect(true);

	// Create listview columns.
	m_lvSocks.InsertColumn(SRC_PORT,     TXT("Local Port"),   75, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(SRC_PROTOCOL, TXT("Protocol"),     75, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(DST_HOST,     TXT("Destination"), 175, LVCFMT_LEFT);
	m_lvSocks.InsertColumn(DST_PORT,     TXT("Remote Port"),  75, LVCFMT_LEFT);

	// Add current socket configs.
	for (uint i = 0; i < m_aoConfigs.size(); ++i)
	{
		CSockConfig* pConfig = m_aoConfigs[i];

		m_lvSocks.InsertItem(i, TXT(""));
		m_lvSocks.ItemPtr(i, pConfig);

		UpdateConfig(i, pConfig);
	}

	// Select 1st by default.
	if (!m_aoConfigs.empty())
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

	// Set ports in use.
	for (uint j = 0; j < m_aoConfigs.size(); ++j)
	{
		CSockConfig* pCfg = m_aoConfigs[j];

		if (pCfg->m_nType == SOCK_STREAM)
			Dlg.m_anTCPPorts.push_back(pCfg->m_nSrcPort);
		else
			Dlg.m_anUDPPorts.push_back(pCfg->m_nSrcPort);
	}

	// Show socket config dialog.
	if (Dlg.RunModal(*this) == IDOK)
	{
		// Add config to collection.
		CSockConfig* pConfig = new CSockConfig;

		*pConfig = Dlg.m_oConfig;

		m_aoConfigs.push_back(pConfig);

		// Add config to view.
		int i = m_lvSocks.ItemCount();

		m_lvSocks.InsertItem(i, TXT(""));
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

	// Set ports in use.
	for (uint j = 0; j < m_aoConfigs.size(); ++j)
	{
		CSockConfig* pCfg = m_aoConfigs[j];

		if (pCfg == pConfig)
			continue;

		if (pCfg->m_nType == SOCK_STREAM)
			Dlg.m_anTCPPorts.push_back(pCfg->m_nSrcPort);
		else
			Dlg.m_anUDPPorts.push_back(pCfg->m_nSrcPort);
	}

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
	size_t nSel = m_lvSocks.Selection();

	// Ignore, if no selection.
	if (nSel == LB_ERR)
		return;

	CSockConfig* pConfig = ItemConfig(nSel);

	// Remove from view and collection.
	m_lvSocks.DeleteItem(nSel);
	Delete(m_aoConfigs, FindIndexOf(m_aoConfigs, pConfig));

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

void CSockOptsDlg::UpdateConfig(size_t nItem, CSockConfig* pConfig)
{
	m_lvSocks.ItemText(nItem, SRC_PORT,     CStrCvt::FormatInt(pConfig->m_nSrcPort));
	m_lvSocks.ItemText(nItem, SRC_PROTOCOL, pConfig->m_strType);
	m_lvSocks.ItemText(nItem, DST_HOST,     pConfig->m_strDstHost);
	m_lvSocks.ItemText(nItem, DST_PORT,     CStrCvt::FormatInt(pConfig->m_nDstPort));
}

/******************************************************************************
** Method:		OnListDoubleClick()
**
** Description:	List item double-clicked. Invoke "Edit | File Props" command.
**
** Parameters:	See WM_NOTIFY.
**
** Returns:		See WM_NOTIFY.
**
*******************************************************************************
*/

LRESULT CSockOptsDlg::OnListDoubleClick(NMHDR& /*oHdr*/)
{
	if (m_lvSocks.IsSelection())
		PostCtrlMsg(BN_CLICKED, IDC_EDIT, Control(IDC_EDIT).Handle());

	return 0;
}
