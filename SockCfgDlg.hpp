/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		SOCKCFGDLG.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CSockCfgDlg class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef SOCKCFGDLG_HPP
#define SOCKCFGDLG_HPP

/******************************************************************************
** 
** The dialog used to edit a socket configuration.
**
*******************************************************************************
*/

class CSockCfgDlg : public CDialog
{
public:
	//
	// Constructors/Destructor.
	//
	CSockCfgDlg();
	
	//
	// Members.
	//
	CSockConfig	m_oConfig;
	
protected:
	//
	// Controls.
	//
	CComboBox	m_cbProtocol;
	CDecimalBox	m_ebSrcPort;
	CEditBox	m_ebDstHost;
	CDecimalBox	m_ebDstPort;
	CEditBox	m_ebSendFile;
	CEditBox	m_ebRecvFile;

	//
	// Message handlers.
	//
	virtual void OnInitDialog();
	virtual bool OnOk();

	void OnCheckHost();

	//
	// Internal methods.
	//
	bool Validate(CEditBox& oEditBox, const char* pszSetting);
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

#endif // SOCKCFGDLG_HPP
