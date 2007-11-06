/******************************************************************************
** (C) Chris Oldwood
**
** MODULE:		TRACEOPTSDLG.HPP
** COMPONENT:	The Application
** DESCRIPTION:	The CTraceOptsDlg class declaration.
**
*******************************************************************************
*/

// Check for previous inclusion
#ifndef TRACEOPTSDLG_HPP
#define TRACEOPTSDLG_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/CommonUI.hpp>

/******************************************************************************
** 
** The dialog used to configure the trace settings.
**
*******************************************************************************
*/

class CTraceOptsDlg : public CDialog
{
public:
	//
	// Constructors/Destructor.
	//
	CTraceOptsDlg();
	
protected:
	//
	// Controls.
	//
	CCheckBox	m_ckTraceConns;
	CCheckBox	m_ckTraceData;
	CCheckBox	m_ckTraceToWindow;
	CEditBox	m_ebTraceLines;
	CCheckBox	m_ckTraceToFile;
	CEditBox	m_ebTraceFile;

	//
	// Message handlers.
	//
	virtual void OnInitDialog();
	virtual bool OnOk();
};

/******************************************************************************
**
** Implementation of inline functions.
**
*******************************************************************************
*/

#endif // TRACEOPTSDLG_HPP
