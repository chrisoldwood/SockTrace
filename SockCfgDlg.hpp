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

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/CommonUI.hpp>
#include "SockConfig.hpp"

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
	
	// Template shorthands.
	typedef std::vector<uint> CUIntArray;

	//
	// Members.
	//
	CSockConfig	m_oConfig;		// The config.
	CUIntArray	m_anTCPPorts;	// TCP Ports already used.
	CUIntArray	m_anUDPPorts;	// UDP Ports already used.
	
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

	void OnResolveHost();

	//
	// Internal methods.
	//
	bool Validate(CEditBox& oEditBox, const tchar* pszSetting);
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

#endif // SOCKCFGDLG_HPP
